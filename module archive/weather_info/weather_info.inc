<?php

function weather_info_get_weather($location, $language) {
	$requestAddress = sprintf('http://weather.yahooapis.com/forecastrss?w=%s&u=c',
		$location);
	try {
		$xml_str = utf8_encode(file_get_contents($requestAddress, 0));
		$weather = new SimplexmlElement($xml_str);
		if ($weather->channel->description == 'Yahoo! Weather - Error') {
			throw new Exception (t("Can't load %loc", array('%loc' => $location)));
		}
		if(!$weather) {
			throw new Exception ('weather failed');
		}
		$fields = array(
			'weather_xml' => $weather->channel->description,
		);
		$hid = db_insert('weather_info')
			->fields($fields)
			->execute();
	} catch (Exception $err) {
		watchdog ('weather_info', 'Cannot get weather for %location: %message',
			array('%location' => $location, '%message' => $err->getMessage()),
			WATCHDOG_ERROR);
		return null;
	}
	return $weather;
}