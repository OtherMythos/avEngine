#ifdef DEBUG_SERVER
#include "VideoRecorder.h"

#include "System/DebugServer/Render/FrameCapture.h"
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
            checkAv(avcodec_send_frame(codec, frame), "send video frame");
            AVPacket* packet = av_packet_alloc();
            if(!packet) throw std::runtime_error("allocate video packet failed");
            while(true){
                int result = avcodec_receive_packet(codec, packet);
                if(result == AVERROR(EAGAIN) || result == AVERROR_EOF) break;
                if(result < 0){
                    av_packet_free(&packet);
                    checkAv(result, "receive video packet");
                }
                if(packet->duration == 0) packet->duration = 1;
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
                              int width, int height, uint64 startTick, int tickRate, std::string& error){
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
        std::error_code fsError;
        if(!std::filesystem::create_directory(outputDir, fsError)){
            error = fsError ? fsError.message() : "output directory must be new";
            return false;
        }
        if(mWorker.joinable()) mWorker.join();
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mQueue.clear();
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
            mQueue.push_back(std::move(frame));
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
                mFramesCaptured, mFramesEncoded, mQueue.size()};
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
            std::unique_ptr<AVFrame, decltype(freeFrame)> image(nullptr, freeFrame);
            std::unique_ptr<SwsContext, decltype(&sws_freeContext)> scaler(nullptr, sws_freeContext);
            AVStream* stream = nullptr;
            uint32 sourceWidth = 0, sourceHeight = 0;
            while(true){
                CapturedFrame frame;
                {
                    std::unique_lock<std::mutex> lock(mMutex);
                    mCv.wait(lock, [this]{ return !mQueue.empty() || mStopping; });
                    if(mQueue.empty()) break;
                    frame = std::move(mQueue.front());
                    mQueue.pop_front();
                }
                mCv.notify_all();
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
                {
                    std::lock_guard<std::mutex> lock(mMutex);
                    ++mFramesEncoded;
                }
            }
            if(codec){
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
