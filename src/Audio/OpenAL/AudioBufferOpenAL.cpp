#include "AudioBufferOpenAL.h"

#include "sndfile.h"

#include "AL/al.h"
#include "AL/alext.h"

#include "Logger/Log.h"
#include "Audio/AudioManager.h"

namespace AV{
    AudioBufferOpenAL::AudioBufferOpenAL(AudioManager* manager) : AudioBuffer(manager) {

    }

    AudioBufferOpenAL::~AudioBufferOpenAL(){

    }

    void AudioBufferOpenAL::load(const std::string& path){
#ifdef DEBUG_SERVER
        if(mManager->debugState().enabled){
            mDebugInfo.lastAttemptPath = path;
            if(!mBufferReady) mDebugInfo.path = path;
        }
#endif
        if(!mManager->isSetup()){
#ifdef DEBUG_SERVER
            debugLoadFailure("audio backend unavailable");
#endif
            return;
        }
        ALenum err, format;
        SNDFILE *sndfile;
        SF_INFO sfinfo = {};
        sf_count_t num_frames;

        /* Open the audio file and check that it's usable. */
        sndfile = sf_open(path.c_str(), SFM_READ, &sfinfo);
        if(!sndfile)
        {
            AV_ERROR("Could not open audio in {}: {}\n", path, sf_strerror(sndfile));
#ifdef DEBUG_SERVER
            debugLoadFailure(sf_strerror(sndfile));
#endif
            return;
        }
        if(sfinfo.channels < 1 || sfinfo.samplerate < 1 || sfinfo.frames < 1 || sfinfo.frames > (sf_count_t)(INT_MAX/sizeof(short))/sfinfo.channels)
        {
#ifdef DEBUG_SERVER
            debugLoadFailure("Invalid sample count, channel count or sample rate");
#endif
            //AV_ERROR("Bad sample count in %s (%" PRId64 ")\n", path, sfinfo.frames);
            sf_close(sndfile);
            return;
        }

        /* Get the sound format, and figure out the OpenAL format */
        format = AL_NONE;
        if(sfinfo.channels == 1)
            format = AL_FORMAT_MONO16;
        else if(sfinfo.channels == 2)
            format = AL_FORMAT_STEREO16;
        else if(sfinfo.channels == 3)
        {
            if(sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
                format = AL_FORMAT_BFORMAT2D_16;
        }
        else if(sfinfo.channels == 4)
        {
            if(sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
                format = AL_FORMAT_BFORMAT3D_16;
        }
        if(!format)
        {
            AV_ERROR("Unsupported channel count: {}\n", sfinfo.channels);
#ifdef DEBUG_SERVER
            debugLoadFailure("Unsupported channel count: " + std::to_string(sfinfo.channels));
#endif
            sf_close(sndfile);
            return;
        }

        /* Decode the whole audio file to a buffer. */
        mMembuf = static_cast<short*>(malloc((size_t)(sfinfo.frames * sfinfo.channels) * sizeof(short)));
        if(!mMembuf){
#ifdef DEBUG_SERVER
            debugLoadFailure("Could not allocate decoded sample buffer");
#endif
            sf_close(sndfile);
            return;
        }

        num_frames = sf_readf_short(sndfile, mMembuf, sfinfo.frames);
        if(num_frames < 1)
        {
#ifdef DEBUG_SERVER
            debugLoadFailure("Could not decode audio samples");
#endif
            free(mMembuf);
            sf_close(sndfile);
            //fprintf(stderr, "Failed to read samples in {} (%" PRId64 ")\n", path, num_frames);
            return;
        }
        mNumBytes = (ALsizei)(num_frames * sfinfo.channels) * (ALsizei)sizeof(short);

        /* Buffer the audio data into a new buffer object, then free the data and
         * close the file.
         */
        ALuint buf = 0;
        alGenBuffers(1, &buf);
        alBufferData(buf, format, mMembuf, mNumBytes, sfinfo.samplerate);

#ifdef DEBUG_SERVER
        AudioBufferDebugInfo decoded;
        if(mManager->debugState().enabled){
            decoded.path = path;
            decoded.lastAttemptPath = path;
            decoded.analyse(mMembuf, num_frames, sfinfo.channels, sfinfo.samplerate);
        }
#endif

        free(mMembuf);
        sf_close(sndfile);

        /* Check if an error occured, and clean up if so. */
        err = alGetError();
        if(err != AL_NO_ERROR)
        {
            AV_ERROR("OpenAL Error: {}", alGetString(err));
#ifdef DEBUG_SERVER
            debugLoadFailure(std::string("OpenAL: ") + alGetString(err));
#endif
            if(buf && alIsBuffer(buf))
                alDeleteBuffers(1, &buf);
            return;
        }

        mBufferReady = true;
        mBuffer = static_cast<unsigned int>(buf);
#ifdef DEBUG_SERVER
        if(mManager->debugState().enabled){
            mDebugInfo = std::move(decoded);
            mManager->debugState().record("loadSucceeded", 0, mDebugId, path);
        }
#endif
    }
}
