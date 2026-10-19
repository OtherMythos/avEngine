#ifdef DEBUG_SERVER
#include "VideoRecorder.h"

#include "System/DebugServer/Render/FrameCapture.h"
#include "System/BaseSingleton.h"
#include "Audio/AudioManager.h"
#include "Logger/Log.h"
#include "OgreRoot.h"

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libswscale/swscale.h>
}

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <cstring>

namespace AV{
    namespace{
        void checkAv(int result, const char* operation){
            if(result >= 0) return;
            char message[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(result, message, sizeof(message));
            throw std::runtime_error(std::string(operation) + ": " + message);
        }

        struct Movie{
            AVFormatContext* context = nullptr;
            ~Movie(){
                if(context){
                    if(context->pb) avio_closep(&context->pb);
                    avformat_free_context(context);
                }
            }
        };

        void writePackets(AVFormatContext* movie, AVCodecContext* codec, AVStream* stream, AVFrame* frame){
            checkAv(avcodec_send_frame(codec, frame), "send encoder frame");
            AVPacket* packet = av_packet_alloc();
            if(!packet) throw std::runtime_error("allocate video packet failed");
            while(true){
                int result = avcodec_receive_packet(codec, packet);
                if(result == AVERROR(EAGAIN) || result == AVERROR_EOF) break;
                if(result < 0){
                    av_packet_free(&packet);
                    checkAv(result, "receive video packet");
                }
                if(packet->duration == 0 && codec->codec_type == AVMEDIA_TYPE_VIDEO) packet->duration = 1;
                av_packet_rescale_ts(packet, codec->time_base, stream->time_base);
                packet->stream_index = stream->index;
                result = av_interleaved_write_frame(movie, packet);
                av_packet_unref(packet);
                if(result < 0){
                    av_packet_free(&packet);
                    checkAv(result, "write video packet");
                }
            }
            av_packet_free(&packet);
        }
    }

    VideoRecorder::VideoRecorder(){
        Ogre::Root::getSingleton().addFrameListener(this);
        mRegistered = true;
    }

    VideoRecorder::~VideoRecorder(){ shutdown(); }

    bool VideoRecorder::start(const std::string& outputDir, int fps, uint64 frameLimit,
                              int width, int height, uint64 startTick, int tickRate, bool engineAudio, std::string& error){
        if(mActive || status().state == "draining"){
            error = "a recording is already active";
            return false;
        }
        if(outputDir.empty() || !std::filesystem::path(outputDir).is_absolute() ||
           fps < 1 || fps > 240 || tickRate % fps != 0 ||
           width < 0 || height < 0 || width > 8192 || height > 8192 ||
           (width == 0) != (height == 0) ||
           (width && ((width & 1) || (height & 1)))){
            error = "output directory must be absolute; fps must divide the fixed update rate; dimensions must be even and at most 8192";
            return false;
        }
        if(engineAudio && (!BaseSingleton::getAudioManager() ||
                           !BaseSingleton::getAudioManager()->simulationSamples())){
            error = "audio=engine requires enabled audio and --simulationAudio at launch";
            return false;
        }
        std::error_code fsError;
        if(!std::filesystem::create_directory(outputDir, fsError)){
            error = fsError ? fsError.message() : "output directory must be new";
            return false;
        }
        if(mWorker.joinable()) mWorker.join();
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mQueue.clear();
            mPendingAudio.clear();
            mOutputPath = (std::filesystem::path(outputDir) / "video.mp4").string();
            mLastError.clear();
            mFps = fps;
            mTicksPerFrame = tickRate / fps;
            mFrameLimit = frameLimit;
            mWidth = width;
            mHeight = height;
            mStartTick = startTick;
            mFramesCaptured = 0;
            mFramesEncoded = 0;
            mAudioSampleFrames = 0;
            mEngineAudio = engineAudio;
            mStopping = false;
            mReachedLimit = false;
            mCapturePending = false;
            mActive = true;
            mState = "recording";
        }
        try{
            mWorker = std::thread([this]{ encode(); });
        }catch(const std::exception& e){
            error = e.what();
            fail(error);
            return false;
        }
        return true;
    }

    void VideoRecorder::stop(){
        if(!mActive) return;
        mActive = false;
        mCapturePending = false;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mStopping = true;
            if(mState == "recording") mState = "draining";
        }
        mCv.notify_all();
    }

    void VideoRecorder::shutdown(){
        stop();
        if(mWorker.joinable()) mWorker.join();
        if(mRegistered){
            Ogre::Root* root = Ogre::Root::getSingletonPtr();
            if(root) root->removeFrameListener(this);
            mRegistered = false;
        }
    }

    int VideoRecorder::ticksUntilFrame(uint64 tick) const{
        if(!mActive) return 0;
        const uint64 elapsed = tick - mStartTick;
        return mTicksPerFrame - static_cast<int>(elapsed % mTicksPerFrame);
    }

    bool VideoRecorder::atFrameBoundary(uint64 tick) const{
        return !mActive || (tick - mStartTick) % mTicksPerFrame == 0;
    }

    void VideoRecorder::prepareFrame(uint64 tick){
        if(!mActive) return;
        const uint64 elapsed = tick - mStartTick;
        mCapturePending = elapsed >= static_cast<uint64>(mTicksPerFrame) &&
            elapsed / mTicksPerFrame > mFramesCaptured;
    }

    void VideoRecorder::audioTick(const std::vector<float>* samples){
        if(!mActive || !mEngineAudio) return;
        if(!samples){
            fail("simulation audio became unavailable during recording");
            return;
        }
        mPendingAudio.insert(mPendingAudio.end(), samples->begin(), samples->end());
    }

    bool VideoRecorder::frameRenderingQueued(const Ogre::FrameEvent&){
        if(!mCapturePending) return true;
        mCapturePending = false;
        CapturedFrame frame;
        std::string error;
        if(!FrameCapture::captureCurrent(frame, error)){
            fail(error);
            return true;
        }
        if((mWidth && (frame.width < static_cast<uint32>(mWidth) || frame.height < static_cast<uint32>(mHeight))) ||
           (!mWidth && ((frame.width & 1) || (frame.height & 1)))){
            fail("render dimensions must be even and at least as large as the requested video dimensions");
            return true;
        }
        std::unique_lock<std::mutex> lock(mMutex);
        mCv.wait(lock, [this]{ return mQueue.size() < 2 || mState == "failed"; });
        if(mState == "failed") return true;
        try{
            QueuedFrame queued;
            queued.image = std::move(frame);
            if(mEngineAudio){
                const uint64 expected = ((mFramesCaptured + 1) * 48000ULL) / mFps;
                const uint64 actual = mAudioSampleFrames + mPendingAudio.size() / 2;
                if(mPendingAudio.size() & 1 || actual + 1 < expected || actual > expected + 1){
                    lock.unlock();
                    fail("audio sample count does not match the video frame boundary");
                    return true;
                }
                queued.audio = std::move(mPendingAudio);
                mPendingAudio.clear();
                mAudioSampleFrames = actual;
            }
            mQueue.push_back(std::move(queued));
        }catch(const std::exception& e){
            lock.unlock();
            fail(e.what());
            return true;
        }
        ++mFramesCaptured;
        if(mFrameLimit && mFramesCaptured == mFrameLimit){
            mReachedLimit = true;
            mStopping = true;
            mActive = false;
            mState = "draining";
        }
        lock.unlock();
        mCv.notify_all();
        return true;
    }

    VideoRecorder::Status VideoRecorder::status() const{
        std::lock_guard<std::mutex> lock(mMutex);
        return {mState, mOutputPath, mLastError, mFps, mFrameLimit,
                mFramesCaptured, mFramesEncoded,
                mEngineAudio ? "engine" : "none", mAudioSampleFrames, mQueue.size()};
    }

    void VideoRecorder::fail(const std::string& error){
        mActive = false;
        std::lock_guard<std::mutex> lock(mMutex);
        mLastError = error;
        mState = "failed";
        mStopping = true;
        mCv.notify_all();
        AV_ERROR("Video recording failed: {}", error);
    }

    void VideoRecorder::encode(){
        try{
            Movie movie;
            auto freeCodec = [](AVCodecContext* value){ avcodec_free_context(&value); };
            auto freeFrame = [](AVFrame* value){ av_frame_free(&value); };
            std::unique_ptr<AVCodecContext, decltype(freeCodec)> codec(nullptr, freeCodec);
            std::unique_ptr<AVCodecContext, decltype(freeCodec)> audioCodec(nullptr, freeCodec);
            std::unique_ptr<AVFrame, decltype(freeFrame)> image(nullptr, freeFrame);
            std::unique_ptr<SwsContext, decltype(&sws_freeContext)> scaler(nullptr, sws_freeContext);
            AVStream* stream = nullptr;
            AVStream* audioStream = nullptr;
            std::vector<float> audioBuffer;
            uint64 audioPts = 0;
            uint32 sourceWidth = 0, sourceHeight = 0;
            auto encodeAudio = [&](int count){
                std::unique_ptr<AVFrame, decltype(freeFrame)> samples(av_frame_alloc(), freeFrame);
                if(!samples) throw std::runtime_error("allocate audio frame failed");
                samples->format = audioCodec->sample_fmt;
                samples->sample_rate = audioCodec->sample_rate;
                checkAv(av_channel_layout_copy(&samples->ch_layout, &audioCodec->ch_layout), "copy audio channels");
                samples->nb_samples = count;
                samples->pts = audioPts;
                checkAv(av_frame_get_buffer(samples.get(), 0), "allocate audio samples");
                for(int i = 0; i < count; ++i){
                    reinterpret_cast<float*>(samples->data[0])[i] = audioBuffer[2 * i];
                    reinterpret_cast<float*>(samples->data[1])[i] = audioBuffer[2 * i + 1];
                }
                writePackets(movie.context, audioCodec.get(), audioStream, samples.get());
                audioPts += count;
                audioBuffer.erase(audioBuffer.begin(), audioBuffer.begin() + 2 * count);
            };
            while(true){
                QueuedFrame queued;
                {
                    std::unique_lock<std::mutex> lock(mMutex);
                    mCv.wait(lock, [this]{ return !mQueue.empty() || mStopping; });
                    if(mQueue.empty()) break;
                    queued = std::move(mQueue.front());
                    mQueue.pop_front();
                }
                mCv.notify_all();
                CapturedFrame& frame = queued.image;
                if(!codec){
                    sourceWidth = frame.width;
                    sourceHeight = frame.height;
                    checkAv(avformat_alloc_output_context2(&movie.context, nullptr, "mp4", mOutputPath.c_str()), "create MP4");
#ifdef __APPLE__
                    const char* encoderName = "h264_videotoolbox";
#else
                    const char* encoderName = "libopenh264";
#endif
                    const AVCodec* encoder = avcodec_find_encoder_by_name(encoderName);
                    if(!encoder) throw std::runtime_error(std::string(encoderName) + " encoder unavailable");
                    codec.reset(avcodec_alloc_context3(encoder));
                    if(!codec) throw std::runtime_error("allocate encoder failed");
                    codec->width = mWidth ? mWidth : frame.width;
                    codec->height = mHeight ? mHeight : frame.height;
                    codec->pix_fmt = AV_PIX_FMT_YUV420P;
                    codec->time_base = AVRational{1, mFps};
                    codec->framerate = AVRational{mFps, 1};
                    codec->bit_rate = 60000000;
                    codec->profile = AV_PROFILE_H264_HIGH;
                    codec->gop_size = mFps;
                    codec->max_b_frames = 0;
                    codec->thread_count = 2;
                    codec->color_range = AVCOL_RANGE_MPEG;
                    codec->colorspace = AVCOL_SPC_BT709;
                    codec->color_primaries = AVCOL_PRI_BT709;
                    codec->color_trc = AVCOL_TRC_IEC61966_2_1;
                    if(movie.context->oformat->flags & AVFMT_GLOBALHEADER) codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
                    checkAv(avcodec_open2(codec.get(), encoder, nullptr), "open H.264 encoder");
                    stream = avformat_new_stream(movie.context, nullptr);
                    if(!stream) throw std::runtime_error("allocate MP4 stream failed");
                    stream->time_base = codec->time_base;
                    checkAv(avcodec_parameters_from_context(stream->codecpar, codec.get()), "copy video parameters");
                    if(mEngineAudio){
                        const AVCodec* aac = avcodec_find_encoder_by_name("aac");
                        if(!aac) throw std::runtime_error("AAC encoder unavailable");
                        audioCodec.reset(avcodec_alloc_context3(aac));
                        if(!audioCodec) throw std::runtime_error("allocate audio encoder failed");
                        audioCodec->sample_rate = 48000;
                        audioCodec->sample_fmt = AV_SAMPLE_FMT_FLTP;
                        audioCodec->time_base = AVRational{1, 48000};
                        audioCodec->bit_rate = 256000;
                        av_channel_layout_default(&audioCodec->ch_layout, 2);
                        if(movie.context->oformat->flags & AVFMT_GLOBALHEADER)
                            audioCodec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
                        checkAv(avcodec_open2(audioCodec.get(), aac, nullptr), "open AAC encoder");
                        audioStream = avformat_new_stream(movie.context, nullptr);
                        if(!audioStream) throw std::runtime_error("allocate MP4 audio stream failed");
                        audioStream->time_base = audioCodec->time_base;
                        checkAv(avcodec_parameters_from_context(audioStream->codecpar, audioCodec.get()), "copy audio parameters");
                    }
                    checkAv(avio_open(&movie.context->pb, mOutputPath.c_str(), AVIO_FLAG_WRITE), "open MP4 output");
                    checkAv(avformat_write_header(movie.context, nullptr), "write MP4 header");
                    image.reset(av_frame_alloc());
                    if(!image) throw std::runtime_error("allocate video frame failed");
                    image->format = codec->pix_fmt;
                    image->width = codec->width;
                    image->height = codec->height;
                    checkAv(av_frame_get_buffer(image.get(), 32), "allocate video pixels");
                    scaler.reset(sws_getContext(sourceWidth, sourceHeight, AV_PIX_FMT_RGB24,
                        codec->width, codec->height, codec->pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr));
                    if(!scaler) throw std::runtime_error("create pixel converter failed");
                    const int* rec709 = sws_getCoefficients(SWS_CS_ITU709);
                    checkAv(sws_setColorspaceDetails(scaler.get(), rec709, 1, rec709, 0,
                        0, 1 << 16, 1 << 16), "set BT.709 conversion");
                }
                if(frame.width != sourceWidth || frame.height != sourceHeight)
                    throw std::runtime_error("render resolution changed during recording");
                checkAv(av_frame_make_writable(image.get()), "make video frame writable");
                const uint8_t* pixels[] = {frame.rgb.data()};
                int stride[] = {static_cast<int>(frame.width * 3)};
                if(sws_scale(scaler.get(), pixels, stride, 0, frame.height, image->data, image->linesize) != codec->height)
                    throw std::runtime_error("incomplete pixel conversion");
                image->pts = mFramesEncoded;
                writePackets(movie.context, codec.get(), stream, image.get());
                if(audioCodec){
                    audioBuffer.insert(audioBuffer.end(), queued.audio.begin(), queued.audio.end());
                    while(audioBuffer.size() >= static_cast<size_t>(audioCodec->frame_size * 2))
                        encodeAudio(audioCodec->frame_size);
                }
                {
                    std::lock_guard<std::mutex> lock(mMutex);
                    ++mFramesEncoded;
                }
            }
            if(codec){
                if(audioCodec){
                    if(!audioBuffer.empty()) encodeAudio(static_cast<int>(audioBuffer.size() / 2));
                    writePackets(movie.context, audioCodec.get(), audioStream, nullptr);
                }
                writePackets(movie.context, codec.get(), stream, nullptr);
                checkAv(av_write_trailer(movie.context), "finalise MP4");
            }
            std::lock_guard<std::mutex> lock(mMutex);
            if(mState != "failed"){
                if(!codec){
                    mState = "failed";
                    mLastError = "no frames were captured";
                }else mState = mReachedLimit ? "completed" : "stopped";
            }
        }catch(const std::exception& e){
            fail(e.what());
        }
    }
}
#endif
