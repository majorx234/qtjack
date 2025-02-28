///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    This file is part of QtJack.                                           //
//    Copyright (C) 2014-2015 Jacob Dawid <jacob@omg-it.works>               //
//                                                                           //
//    QtJack is free software: you can redistribute it and/or modify         //
//    it under the terms of the GNU General Public License as published by   //
//    the Free Software Foundation, either version 3 of the License, or      //
//    (at your option) any later version.                                    //
//                                                                           //
//    QtJack is distributed in the hope that it will be useful,              //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of         //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
//    GNU General Public License for more details.                           //
//                                                                           //
//    You should have received a copy of the GNU General Public License      //
//    along with QtJack. If not, see <http://www.gnu.org/licenses/>.         //
//                                                                           //
//    It is possible to obtain a closed-source license of QtJack.            //
//    If you're interested, contact me at: jacob@omg-it.works                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// Own includes:
#include "processor.h"
#include "client.h"

// Standard includes
#include <cstdlib>

// Qt includes
#include <QStringList>
#include <QDebug>

namespace QtJack {

Client::Client(QObject *parent) :
    QObject(parent),
    _processor(0) {
    _jackClient = 0;
}

Client::~Client() {
    disconnectFromServer();
}

bool Client::connectToServer(QString name) {
    if(_jackClient) {
        // Already connected
        return false;
    }

    if((_jackClient = jack_client_open(name.toStdString().c_str(), JackNullOption, NULL)) == 0) {
        return false;
    } else {
        // Set callbacks
        jack_set_thread_init_callback(_jackClient, Client::threadInitCallback, (void*)this);
        jack_set_process_callback(_jackClient, Client::processCallback, (void*)this);
        jack_set_freewheel_callback(_jackClient, Client::freewheelCallback, (void*)this);
        jack_set_client_registration_callback(_jackClient, Client::clientRegistrationCallback, (void*)this);
        jack_set_port_registration_callback(_jackClient, Client::portRegistrationCallback, (void*)this);
        jack_set_port_connect_callback(_jackClient, Client::portConnectCallback, (void*)this);
        //jack_set_port_rename_callback(_jackClient, Client::portRenameCallback, (void*)this);
        jack_set_graph_order_callback(_jackClient, Client::graphOrderCallback, (void*)this);
        jack_set_latency_callback(_jackClient, Client::latencyCallback, (void*)this);
        jack_set_buffer_size_callback(_jackClient, Client::bufferSizeCallback, (void*)this);
        jack_set_sample_rate_callback(_jackClient, Client::sampleRateCallback, (void*)this);
        jack_set_xrun_callback(_jackClient, Client::xrunCallback, (void*)this);
        jack_on_shutdown(_jackClient, Client::shutdownCallback, (void*)this);
        jack_on_info_shutdown(_jackClient, Client::infoShutdownCallback, (void*)this);

        Q_EMIT connectedToServer();
        return true;
    }
}

bool Client::disconnectFromServer() {
    if(!_jackClient) {
        // Already disconnected
        return false;
    }

    bool success = (jack_deactivate(_jackClient) == 0
                 && jack_client_close(_jackClient) == 0);
    _jackClient = 0;
    Q_EMIT disconnectedFromServer();

    return success;
}

AudioPort Client::registerAudioOutPort(QString name) {
    if(!_jackClient) {
        return AudioPort();
    }

    AudioPort audioPort = AudioPort(jack_port_register(
                                        _jackClient,
                                        name.toStdString().c_str(),
                                        JACK_DEFAULT_AUDIO_TYPE,
                                        JackPortIsOutput, 0));
    return audioPort;
}

AudioPort Client::registerAudioInPort(QString name) {
    if(!_jackClient) {
        return AudioPort();
    }

    AudioPort audioPort = AudioPort(jack_port_register(
                                        _jackClient,
                                        name.toStdString().c_str(),
                                        JACK_DEFAULT_AUDIO_TYPE,
                                        JackPortIsInput, 0));
    return audioPort;
}

MidiPort Client::registerMidiOutPort(QString name) {
    if(!_jackClient) {
        return MidiPort();
    }

    MidiPort midiPort = MidiPort(jack_port_register(
                                     _jackClient,
                                     name.toStdString().c_str(),
                                     JACK_DEFAULT_MIDI_TYPE,
                                     JackPortIsOutput, 0));
    return midiPort;
}

MidiPort Client::registerMidiInPort(QString name) {
    if(!_jackClient) {
        return MidiPort();
    }

    MidiPort midiPort = MidiPort(jack_port_register(
                                     _jackClient,
                                     name.toStdString().c_str(),
                                     JACK_DEFAULT_MIDI_TYPE,
                                     JackPortIsInput, 0));
    return midiPort;
}

bool Client::connect(AudioPort source, AudioPort destination) {
    if(!_jackClient) {
        return false;
    }

    int result = jack_connect(_jackClient,
                              source.fullName().toStdString().c_str(),
                              destination.fullName().toStdString().c_str());
    return result == 0;
}

bool Client::connect(MidiPort source, MidiPort destination) {
    if(!_jackClient) {
        return false;
    }

    int result = jack_connect(_jackClient,
                              source.fullName().toStdString().c_str(),
                              destination.fullName().toStdString().c_str());
    return result == 0;
}

bool Client::disconnect(AudioPort source, AudioPort destination) {
    if(!_jackClient) {
        return false;
    }

    int result = jack_disconnect(_jackClient,
                                 source.fullName().toStdString().c_str(),
                                 destination.fullName().toStdString().c_str());
    return result == 0;
}

bool Client::disconnect(MidiPort source, MidiPort destination) {
    if(!_jackClient) {
        return false;
    }

    int result = jack_disconnect(_jackClient,
                                 source.fullName().toStdString().c_str(),
                                 destination.fullName().toStdString().c_str());
    return result == 0;
}

QStringList Client::clientList() const {
    if(!_jackClient) {
        return QStringList();
    }

    QStringList clientList;
    const char **ports = jack_get_ports(_jackClient, 0, 0, 0);
    for(int i = 0; ports && ports[i]; ++i) {
        QString clientName = Port(jack_port_by_name(_jackClient, ports[i])).clientName();
        if(!clientList.contains(clientName)) {
            clientList.append(clientName);
        }
    }

    if(ports) {
        jack_free(ports);
    }

    return clientList;
}

QList<Port> Client::portsForClient(QString clientName) const {
    if(!_jackClient) {
        return QList<Port>();
    }

    QList<Port> portList;
    const char **ports = jack_get_ports(_jackClient, 0, 0, 0);
    for(int i = 0; ports && ports[i]; ++i) {
        Port port(jack_port_by_name(_jackClient, ports[i]));
        if(port.clientName() == clientName) {
            portList.append(port);
        }
    }

    if(ports) {
        jack_free(ports);
    }

    return portList;
}

bool Client::activate() {
    if(!_jackClient) {
        return false;
    }

    if(jack_activate(_jackClient) == 0) {
        Q_EMIT activated();
        return true;
    }
    return false;
}

bool Client::deactivate() {
    if(!_jackClient) {
        return false;
    }

    if(jack_deactivate(_jackClient) == 0) {
        Q_EMIT deactivated();
        return true;
    }
    return false;
}

bool Client::startTransport() {
    if(_jackClient) {
        jack_transport_start(_jackClient);
        return true;
    }
    return false;
}

bool Client::stopTransport() {
    if(_jackClient) {
        jack_transport_stop(_jackClient);
        return true;
    }
    return false;
}

int Client::sampleRate() const {
    if(!_jackClient) {
        return -1;
    }
    return jack_get_sample_rate(_jackClient);
}

int Client::bufferSize() const {
    if(!_jackClient) {
        return -1;
    }
    return jack_get_buffer_size(_jackClient);
}

float Client::cpuLoad() const {
    if(!_jackClient) {
        return 0.0;
    }
    return jack_cpu_load(_jackClient);
}

bool Client::isRealtime() const {
    if(!_jackClient) {
        return false;
    }

    return jack_is_realtime(_jackClient) == 1;
}

int Client::numberOfInputPorts(QString clientName) const {
    QList<Port> ports = portsForClient(clientName);
    int inputPortCount = 0;
    Q_FOREACH(Port port, ports) {
        if(port.isInput()) {
            inputPortCount++;
        }
    }
    return inputPortCount;
}

int Client::numberOfOutputPorts(QString clientName) const {
    QList<Port> ports = portsForClient(clientName);
    int outputPortCount = 0;
    Q_FOREACH(Port port, ports) {
        if(port.isOutput()) {
            outputPortCount++;
        }
    }
    return outputPortCount;
}


Port Client::portByName(QString name) {
    if(!_jackClient) {
        return Port();
    }

    return Port(jack_port_by_name(_jackClient, name.toStdString().c_str()));
}

Port Client::portById(int id) {
    if(!_jackClient) {
        return Port();
    }

    return Port(jack_port_by_id(_jackClient, id));
}

TransportState Client::transportState() {
    if(!_jackClient) {
        return TransportStateUnknown;
    }
    jack_transport_state_t jackTransportState = jack_transport_query(_jackClient, 0);
    switch (jackTransportState) {
        case JackTransportStopped: return TransportStateStopped; break;
        case JackTransportRolling: return TransportStateRolling; break;
        case JackTransportLooping: return TransportStateLooping; break;
        case JackTransportStarting: return TransportStateStarting; break;
        default: break;
    }
    return TransportStateUnknown;
}

TransportPosition Client::queryTransportPosition() {
    if(!_jackClient) {
        return TransportPosition();
    }
    jack_position_t jackPosition;
    jack_transport_query(_jackClient, &jackPosition);

    return TransportPosition(jackPosition);
}

bool Client::requestTransportReposition(TransportPosition transportPosition) {
    if(!_jackClient) {
        return false;
    }

    jack_position_t jackPosition = transportPosition.toJackPosition();
    return jack_transport_reposition(_jackClient, &jackPosition) == 0;
}

void Client::setMainProcessor(Processor *audioProcessor) {
    _processor = audioProcessor;
}

void Client::threadInit() {
}

void Client::process(int samples) {
    if(_processor) {
        _processor->process(samples);
    }
}

void Client::freewheel(int starting) {
    if(starting == 0) {
        Q_EMIT stoppedFreewheeling();
    } else {
        Q_EMIT startedFreewheeling();
    }
}

void Client::clientRegistration(const char *name, int reg) {
    if(reg == 0) {
        Q_EMIT clientUnregistered(QString(name));
    } else {
        Q_EMIT clientRegistered(QString(name));
    }
}

void Client::portRegistration(jack_port_id_t portId, int reg) {
    QtJack::Port port(jack_port_by_id(_jackClient, portId));
    if(port.isValid()) {
        if(reg == 0) {
            Q_EMIT portUnregistered(port);
        } else {
            Q_EMIT portRegistered(port);
        }
    }
}

void Client::portConnect(jack_port_id_t a, jack_port_id_t b, int connect) {
    QtJack::Port portA(jack_port_by_id(_jackClient, a));
    QtJack::Port portB(jack_port_by_id(_jackClient, b));

    if(portA.isValid() && portB.isValid()) {
        if(connect == 0) {
            Q_EMIT portsDisconnected(portA, portB);
        } else {
            Q_EMIT portsConnected(portA, portB);
        }
    }
}

void Client::portRename(jack_port_id_t portId, const char *oldName, const char *newName) {
    QtJack::Port port(jack_port_by_id(_jackClient, portId));
    if(port.isValid()) {
        Q_EMIT portRenamed(port, QString(oldName), QString(newName));
    }
}

void Client::graphOrder() {
    Q_EMIT graphOrderHasChanged();
}

void Client::latency(jack_latency_callback_mode_t mode) {
    Q_UNUSED(mode);
}

void Client::sampleRate(int samples) {
    Q_EMIT sampleRateChanged(samples);
}

void Client::bufferSize(int samples) {
    Q_EMIT bufferSizeChanged(samples);
}

void Client::xrun() {
    Q_EMIT xrunOccured();
}

void Client::shutdown() {
    Q_EMIT disconnectFromServer();
    Q_EMIT serverShutdown();
}

void Client::infoShutdown(jack_status_t code, const char *reason) {
    Q_UNUSED(code);
    Q_UNUSED(reason);
}

// Static callbacks

int Client::processCallback(jack_nframes_t sampleCount,
                            void *argument) {
    Client *jackClient = static_cast<Client*>(argument);
    if(jackClient) {
        jackClient->process(sampleCount);
    }
    return 0;
}

void Client::threadInitCallback(void *argument) {
    Client *jackClient = static_cast<Client*>(argument);
    if(jackClient) {
        jackClient->threadInit();
    }
}

void Client::freewheelCallback(int starting, void *argument) {
    Client *jackClient = static_cast<Client*>(argument);
    if(jackClient) {
        jackClient->freewheel(starting);
    }
}

void Client::clientRegistrationCallback(const char* name, int reg, void *argument) {
    Client *jackClient = static_cast<Client*>(argument);
    if(jackClient) {
        jackClient->clientRegistration(name, reg);
    }
}

void Client::portRegistrationCallback(jack_port_id_t port, int reg, void *argument) {
    Client *jackClient = static_cast<Client*>(argument);
    if(jackClient) {
        jackClient->portRegistration(port, reg);
    }
}

void Client::portConnectCallback(jack_port_id_t a, jack_port_id_t b, int connect, void* argument) {
    Client *jackClient = static_cast<Client*>(argument);
    if(jackClient) {
        jackClient->portConnect(a, b, connect);
    }
}

void Client::portRenameCallback(jack_port_id_t port, const char* oldName, const char* newName, void *argument) {
    Client *jackClient = static_cast<Client*>(argument);
    if(jackClient) {
        jackClient->portRename(port, oldName, newName);
    }
}

int Client::graphOrderCallback(void *argument) {
    Client *jackClient = static_cast<Client*>(argument);
    if(jackClient) {
        jackClient->graphOrder();
    }
    return 0;
}

void Client::latencyCallback(jack_latency_callback_mode_t mode, void *argument) {
    Client *jackClient = static_cast<Client*>(argument);
    if(jackClient) {
        jackClient->latency(mode);
    }
}

int Client::sampleRateCallback(jack_nframes_t sampleCount,
                               void *argument) {
    Client *jackClient = static_cast<Client*>(argument);
    if(jackClient) {
        jackClient->sampleRate(sampleCount);
    }
    return 0;
}

int Client::bufferSizeCallback(jack_nframes_t bufferSize,
                               void *argument) {
    Client *jackClient = static_cast<Client*>(argument);
    if(jackClient) {
        jackClient->bufferSize(bufferSize);
    }
    return 0;
}

int Client::xrunCallback(void *argument) {
    Client *jackClient = static_cast<Client*>(argument);
    if(jackClient) {
        jackClient->xrun();
    }
    return 0;
}

void Client::shutdownCallback(void *argument) {
    Client *jackClient = static_cast<Client*>(argument);
    if(jackClient) {
        jackClient->shutdown();
    }
}

void Client::infoShutdownCallback(jack_status_t code, const char* reason, void *argument) {
    Client *jackClient = static_cast<Client*>(argument);
    if(jackClient) {
        jackClient->infoShutdown(code, reason);
    }
}

double Client::getJackTimeInMs() {
    double sampleRate = jack_get_sample_rate(_jackClient);
    jack_nframes_t nframes = jack_last_frame_time(_jackClient);
    return (nframes * 1000.0) / sampleRate;
}

int Client::getJackTime() {
    jack_nframes_t nframes = jack_last_frame_time(_jackClient);
    return nframes;
}

int Client::getJackFrameTime() {
    jack_nframes_t nframes = jack_frame_time(_jackClient);
    return nframes;
}
} // namespace QtJack
