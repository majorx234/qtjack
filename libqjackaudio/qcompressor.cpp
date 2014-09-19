///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    This file is part of QJackAudio.                                       //
//    Copyright (C) 2014 Jacob Dawid, jacob@omg-it.works                     //
//                                                                           //
//    QJackAudio is free software: you can redistribute it and/or modify     //
//    it under the terms of the GNU General Public License as published by   //
//    the Free Software Foundation, either version 3 of the License, or      //
//    (at your option) any later version.                                    //
//                                                                           //
//    QJackAudio is distributed in the hope that it will be useful,          //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of         //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
//    GNU General Public License for more details.                           //
//                                                                           //
//    You should have received a copy of the GNU General Public License      //
//    along with QJackAudio. If not, see <http://www.gnu.org/licenses/>.     //
//                                                                           //
//    It is possible to obtain a closed-source license of QJackAudio.        //
//    If you're interested, contact me at: jacob@omg-it.works                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// Qt includes
#include <QMutexLocker>

// Own includes
#include <QCompressor>

QCompressor::QCompressor(QObject *parent)
    : QDigitalFilter(parent)
{
    _threshold = 0.0;
    _ratio = 1.0;
    _attack = 0.0;
    _release = 0.0;
    _inputGain = 0.0;
    _makeupGain = 0.0;
    _bypass = false;
}

QCompressor::~QCompressor()
{
}

void QCompressor::process(QSampleBuffer sampleBuffer)
{
    bool isClipping = false;
    bool isActive = false;

    _mutex.lock();
    double threshold = _threshold;
    double ratio = _ratio;
    double attack = _attack;
    double release = _release;
    double inputGain = _inputGain;
    double makeupGain = _makeupGain;
    bool bypass = _bypass;
    _mutex.unlock();

    if(bypass) {
        return;
    }

    // TODO: To be implemented.
    Q_UNUSED(attack);
    Q_UNUSED(release);

    int bufferSize = sampleBuffer.bufferSize();
    double inputGainMultiplier = QUnits::dbToLinear(inputGain);
    double makeupGainMultiplier = QUnits::dbToLinear(makeupGain);
    for(int i = 0; i < bufferSize; i++) {
        // Read audio sample
        double sample = sampleBuffer.readAudioSample(i) * inputGainMultiplier;
        // Determine peak in dB
        double peakDb = QUnits::linearToDb(QUnits::peak(sample));

        double resultSample = sample;

        // Check if peak is over threshold
        if(peakDb > threshold) {
            // Perform signal compression
            isActive = true;
            double dbOverThreshold = peakDb - threshold;
            double dbOverThresholdCompressed = dbOverThreshold / ratio;
            double dbResultingPeak = threshold + dbOverThresholdCompressed;

            resultSample = QUnits::dbToLinear(dbResultingPeak * ( sample > 0.0 ? 1.0 : -1.0 ));
        }

        double result = resultSample * makeupGainMultiplier;

        if(result > 1.0) {
            result = 1.0;
            isClipping = true;
        }

        if(result < -1.0) {
            result = -1.0;
            isClipping = true;
        }

        sampleBuffer.writeAudioSample(i, result);
    }

    if(isClipping) {
        emit clipping();
    }

    if(isActive) {
        emit active();
    }
}

double QCompressor::threshold()
{
    return _threshold;
}

double QCompressor::ratio()
{
    return _ratio;
}

double QCompressor::attack()
{
    return _attack;
}

double QCompressor::release()
{
    return _release;
}

double QCompressor::inputGain()
{
    return _inputGain;
}

double QCompressor::makeupGain()
{
    return _makeupGain;
}

bool QCompressor::bypass()
{
    return _bypass;
}

void QCompressor::setThreshold(double threshold)
{
    QMutexLocker mutexLocker(&_mutex);
    _threshold = threshold;
    emit thresholdChanged(_threshold);
}

void QCompressor::setRatio(double ratio)
{
    QMutexLocker mutexLocker(&_mutex);
    _ratio = ratio;
    emit ratioChanged(_ratio);
}

void QCompressor::setAttack(double attack)
{
    QMutexLocker mutexLocker(&_mutex);
    _attack = attack;
    emit attackChanged(_attack);
}

void QCompressor::setRelease(double release)
{
    QMutexLocker mutexLocker(&_mutex);
    _release = release;
    emit releaseChanged(_release);
}

void QCompressor::setInputGain(double inputGain)
{
    QMutexLocker mutexLocker(&_mutex);
    _inputGain = inputGain;
    emit inputGainChanged(_inputGain);
}

void QCompressor::setMakeupGain(double makeupGain)
{
    QMutexLocker mutexLocker(&_mutex);
    _makeupGain = makeupGain;
    emit makeupGainChanged(_makeupGain);
}

void QCompressor::setBypass(bool bypass)
{
    QMutexLocker mutexLocker(&_mutex);
   _bypass = bypass;
   emit bypassChanged(bypass);
}
