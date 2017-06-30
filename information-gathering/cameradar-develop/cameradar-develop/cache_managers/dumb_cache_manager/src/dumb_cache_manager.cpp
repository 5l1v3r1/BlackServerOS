// Copyright 2016 Etix Labs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <dumb_cache_manager.h>

namespace etix {
namespace cameradar {

const std::string dumb_cache_manager::name = "dumb-cache-manager";

dumb_cache_manager::~dumb_cache_manager() {}

const std::string&
dumb_cache_manager::get_name() const {
    return dumb_cache_manager::static_get_name();
}

const std::string&
dumb_cache_manager::static_get_name() {
    return dumb_cache_manager::name;
}

bool
dumb_cache_manager::configure(std::shared_ptr<etix::cameradar::configuration> configuration) {
    return this->load_dumb_conf(configuration);
}

bool
dumb_cache_manager::load_dumb_conf(std::shared_ptr<etix::cameradar::configuration> configuration) {
    this->configuration = configuration;

    return true;
}

//! Replaces all cached streams by the content of the vector given as
//! parameter
void
dumb_cache_manager::set_streams(std::vector<etix::cameradar::stream_model> model) {
    std::lock_guard<std::mutex> lock(m);
    this->streams = model;
}

//! Inserts a single stream to the cache
void
dumb_cache_manager::update_stream(const etix::cameradar::stream_model& newmodel) {
    std::lock_guard<std::mutex> lock(m);
    for (auto& stream : this->streams) {
        if (stream.address == newmodel.address && stream.port == newmodel.port) {
            stream = newmodel;
        }
    }
}

//! Gets all cached streams
std::vector<etix::cameradar::stream_model>
dumb_cache_manager::get_streams() {
    std::vector<stream_model> ret;
    for (const auto& stream : this->streams) {
        if (not stream.service_name.compare("rtsp") && not stream.state.compare("open"))
            ret.push_back(stream);
    }
    return ret;
}

//! Gets all valid streams
std::vector<etix::cameradar::stream_model>
dumb_cache_manager::get_valid_streams() {
    std::vector<stream_model> ret;
    for (const auto& stream : this->streams) {
        if (stream.ids_found && stream.path_found) ret.push_back(stream);
    }
    return ret;
}

// Returns true if the stream passed as a parameter has changed in the cache
bool
dumb_cache_manager::has_changed(const etix::cameradar::stream_model& old) {
    for (const auto& stream : this->streams) {
        if (stream.address == old.address)
            if (stream.path_found != old.path_found || stream.ids_found != old.ids_found)
                return true;
    }
    return false;
}

extern "C" {
cache_manager_iface*
cache_manager_instance_new() {
    return new dumb_cache_manager();
}
}
}
}
