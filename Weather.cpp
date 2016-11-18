//
// Created by ziyang on 11/17/16.
//
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <curl.h>
#include "pugixml.hpp"


#include "Weather.h"

size_t AppendDataToStringCurlCallback(void *ptr, size_t size, size_t nmemb, void *vstring) {
	std::string *pstring = (std::string *) vstring;
	pstring->append((char *)ptr, size * nmemb);
	return size * nmemb;
}

bool Weather::GetWeatherFromNatWeatherService() {

	CURL *curl_handle;
	const std::string url_ann_arbor = "http://forecast.weather.gov/MapClick.php?textField1=42.28&textField2=-83.74&FcstType=dwml";

	curl_global_init(CURL_GLOBAL_ALL);

	//init the curl session 
	curl_handle = curl_easy_init();

	//set URL to get here 
	curl_easy_setopt(curl_handle, CURLOPT_URL, url_ann_arbor.c_str());

	//Switch on full protocol/debug output while testing 
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);

	//disable progress meter, set to 0L to enable and disable debug output 
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);

	//set user agent 
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0");

	//write data to string 
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, AppendDataToStringCurlCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &body_);

	//get it! 
	curl_easy_perform(curl_handle);

	//cleanup curl stuff 
	curl_easy_cleanup(curl_handle);

	return true;
}

bool Weather::ParseXml() {

	//load string into XML DOM
	pugi::xml_parse_result result = xmldoc_.load(body_.c_str());

	if (result) {
		std::cout << "XML parsed without errors" << std::endl;
	}
	else {
		std::cout << "XML parsed with errors" << std::endl;
		std::cout << "Error description: " << result.description() << "\n";
	}

	return true;
}

bool Weather::GetCurrentWeather() {

    //this is the top level node
    pugi::xml_node rootnode = xmldoc_.child("dwml");

    //search all child for current observations
    for (pugi::xml_node it = rootnode.first_child(); it; it = it.next_sibling()) {

        if (it.attribute("type").as_string() == "current observations") {

            //read current temperature
            ReadXmlChildValue(weather_data_.temperature, it.child("parameters").child("temperature"));

            //read current  weather conditions
            weather_data_.weather_conditions = it.child("parameters")
                    .child("weather").child("weather-conditions")
                    .attribute("weather-summary").as_string();

            //read wind speed
            ReadXmlChildValue(weather_data_.wind_speed, it.child("parameters")
                    .child("wind-speed").child("value"));
        }
    }

    return true;
}

bool Weather::PrintCurrentWeather() {
    std::cout << weather_data_.temperature << std::endl
              << weather_data_.weather_conditions << std::endl
              << weather_data_.wind_speed << std::endl;
}