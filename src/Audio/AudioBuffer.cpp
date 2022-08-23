#include "AudioBuffer.h"

#include "sndfile.h"

#include "AL/al.h"
#include "AL/alext.h"

#include "Logger/Log.h"

namespace AV{
    AudioBuffer::AudioBuffer()
        : mBufferReady(false),
          mBuffer(0) {

    }

    AudioBuffer::~AudioBuffer(){

    }

    void AudioBuffer::play(){

    }

    void AudioBuffer::pause(){

    }

    void AudioBuffer::load(const std::string& path){
        ALenum err, format;
        SNDFILE *sndfile;
        SF_INFO sfinfo;
        sf_count_t num_frames;

        /* Open the audio file and check that it's usable. */
        sndfile = sf_open(path.c_str(), SFM_READ, &sfinfo);
        if(!sndfile)
        {
            AV_ERROR("Could not open audio in {}: {}\n", path, sf_strerror(sndfile));
            return;
        }
        if(sfinfo.frames < 1 || sfinfo.frames > (sf_count_t)(INT_MAX/sizeof(short))/sfinfo.channels)
        {
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
            sf_close(sndfile);
            return;
        }

        /* Decode the whole audio file to a buffer. */
        mMembuf = static_cast<short*>(malloc((size_t)(sfinfo.frames * sfinfo.channels) * sizeof(short)));

        num_frames = sf_readf_short(sndfile, mMembuf, sfinfo.frames);
        if(num_frames < 1)
        {
            free(mMembuf);
            sf_close(sndfile);
            //fprintf(stderr, "Failed to read samples in {} (%" PRId64 ")\n", path, num_frames);
            return;
        }
        mNumBytes = (ALsizei)(num_frames * sfinfo.channels) * (ALsizei)sizeof(short);

        /* Buffer the audio data into a new buffer object, then free the data and
         * close the file.
         */
        alGenBuffers(1, &mBuffer);
        alBufferData(mBuffer, format, mMembuf, mNumBytes, sfinfo.samplerate);

        free(mMembuf);
        sf_close(sndfile);

        /* Check if an error occured, and clean up if so. */
        err = alGetError();
        if(err != AL_NO_ERROR)
        {
            AV_ERROR("OpenAL Error: {}", alGetString(err));
            if(mBuffer && alIsBuffer(mBuffer))
                alDeleteBuffers(1, &mBuffer);
            return;
        }

        mBufferReady = true;
    }
}
