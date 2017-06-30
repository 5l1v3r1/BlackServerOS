/******************************************************************************/
/*
 *
 * Copyright 2012-2016 AOL Inc. All rights reserved.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this Software except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
'use strict';

var util           = require('util')
  , simpleSource   = require('./simpleSource.js')
  , request        = require('request')
  ;

//////////////////////////////////////////////////////////////////////////////////
function URLSource (api, section) {
  URLSource.super_.call(this, api, section);
  var self = this;

  self.url          = api.getConfig(section, "url");
  self.reload       = +api.getConfig(section, "reload", -1);
  self.headers      = {};
  var headers       = api.getConfig(section, "headers");
  this.cacheTimeout = -1;

  if (this.url === undefined) {
    console.log(this.section, "- ERROR not loading since no url specified in config file");
    return;
  }

  if (headers) {
    headers.split(";").forEach(function(header) {
      var parts = header.split(":");
      if (parts.length === 2) {
        self.headers[parts[0].trim()] = parts[1].trim();
      }
    });
  }

  if (!this.initSimple())
    return;

  setImmediate(this.load.bind(this));

  // Reload url every so often
  if (this.reload > 0) {
    setInterval(this.load.bind(this), this.reload*1000*60);
  }
}
util.inherits(URLSource, simpleSource);
//////////////////////////////////////////////////////////////////////////////////
URLSource.prototype.simpleSourceLoad = function(setFunc, cb) {
  var self = this;

  request(self.url, {headers: self.headers}, function (error, response, body) {
    if (!error && response.statusCode === 200) {
      self.parse(body, setFunc, cb);
    } else {
      cb(error);
    }
  });
};
//////////////////////////////////////////////////////////////////////////////////
exports.initSource = function(api) {
  var sections = api.getConfigSections().filter(function(e) {return e.match(/^url:/);});
  sections.forEach(function(section) {
    var source = new URLSource(api, section);
  });
};
//////////////////////////////////////////////////////////////////////////////////
