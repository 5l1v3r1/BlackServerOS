#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#   Belati is tool for Collecting Public Data & Public Document from Website and other service for OSINT purpose.
#   This tools is inspired by Foca and Datasploit for OSINT
#   Copyright (C) 2017  cacaddv@gmail.com (Petruknisme a.k.a Aan Wahyu)

#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 2 of the License, or
#    (at your option) any later version.

#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

# This file is part of Belati project

import re,sys
from bs4 import BeautifulSoup
from database import Database
from logger import Logger
from url_request import URLRequest

# Console color
G = '\033[92m'  # green
Y = '\033[93m'  # yellow
B = '\033[94m'  # blue
R = '\033[91m'  # red
W = '\033[0m'   # white

url_req = URLRequest()
log = Logger()

class GatherCompany(object):
    def crawl_company_employee(self, company_name, proxy_address, project_id):
        self.db = Database()
        self.project_id = project_id
        self.company_id = 0
        comp_strip = company_name.replace(" ", "+")
        url = 'https://www.google.com/search?q={}+site:linkedin.com&num=200'.format(comp_strip)

        data = url_req.standart_request(url, proxy_address)

        soup = BeautifulSoup( data, 'html.parser' )
        company_linkedin_url_list = []

        #Getting all h3 tags with class 'r'
        scrap_container = soup.find_all('div', class_='rc')
        for rc in scrap_container:
            soup2 = BeautifulSoup( str(rc), 'html.parser' )
            url = soup2.find_all('h3', class_= 'r')
            url_fix = re.findall(r'http[s]?://(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\(\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))+', str(url))
            linkedin_url = re.findall(r'(http[s]?://.*\.linkedin\.com/in/.*)', str(url_fix).strip("\'[]")) # filter only *.linked.com
            company_linkedin_url = re.findall(r'(http[s]?://.*\.linkedin\.com/company/.*)', str(url_fix).strip("\'[]")) # filter only *.linked.com/company
            job_title = soup2.find_all('div', class_='slp f')
            url_tag = soup2.find_all("a")[0].string

            # Check if URL is match with one of the string from company name(?)
            if company_linkedin_url:
                is_contain_name = 0
                for x in company_name.split():
                    if x in url_tag:
                        print(" X" + str(x) + "url tag " + str(url_tag))
                        is_contain_name = 1
                        break

                if is_contain_name == 1:
                    company_linkedin_url_list.append(company_linkedin_url)
                    self.company_id = self.db.insert_linkedin_company_info(self.project_id, str(company_name), str(company_linkedin_url), "Lorem ipsum")

            # Get data when linkedin url is like this : *.linkedin.com/in
            if not linkedin_url:
                pass
            else:
                name_result = re.sub('<[^<]+?>', '', str(rc.h3.a)) # strip all html tags like <em>
                job_title_result = re.sub('<[^<]+?>', '', str(job_title)) # strip all html tags like <em>
                name_fix = str(name_result.replace('| LinkedIn', ''))
                job_title_fix   = str(job_title_result.replace('\u200e', ' ')).strip("\'[]")
                linkedin_url_fix = str(linkedin_url).strip("\'[]")
                log.console_log("{}[+] --------------------------------------------------- [+]{}".format(Y, W))
                log.console_log("Name: {}".format( name_fix ))
                log.console_log("Job Title: {}".format( job_title_fix ))
                log.console_log("Url: {}".format( linkedin_url_fix ))
                log.console_log("{}[+] --------------------------------------------------- [+]{}\n".format(Y, W))

                self.db.insert_company_employees(self.project_id, name_fix, job_title_fix, linkedin_url_fix)

        log.console_log("\n\n{}[+] --------------------------------------------------- [+]{}".format(Y, W))
        log.console_log("{}[+] Found LinkedIn Company URL: {}".format(Y, W))
        for url in company_linkedin_url_list:
            log.console_log("{} {} {}".format(Y, str(url), W))
