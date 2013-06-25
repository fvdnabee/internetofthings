<?php

/**
 * Adds the necessary JavaScript files.
 */
function _coap_resource_add_js(){
	drupal_add_js('https://www.google.com/jsapi');
	drupal_add_js(drupal_get_path('module', 'coap_sensor') . '/js/coap_resource.js');	
	drupal_add_js(array('coap_resource' => array('module_path' => drupal_get_path('module', 'coap_sensor'))), array('type' => 'setting'));	
}

/**
 * Adds the given response to the database table with values for the given user.
 */
function coap_resource_insert_response_into_database($response_obj, $uid){
	//	Parsing values from response
	$max_age = $response_obj->get_max_age();
	$value = $response_obj->get_payload_text();
	$format = $response_obj->get_content_format();
	$content_type = $response_obj->get_message_code();
	if($format == 'text/plain'){
		$value = coap_resource_get_temperature($value);
	}
	$hid = db_insert('coap_sensor_value')
		->fields(array(
			'payload' => $response_obj->get_payload(),
			'content_format' => $format,
			'original_response' => $response_obj->get_hex_message(),
			'parsed_value' => $value,
			'max_age' => $max_age,
			'uri' => ($response_obj->get_ip() . '/' . $response_obj->get_resource()),
			'uid' => $uid,
		))
		->execute();
}

/**
 * Returns the number of observers that are observing the given resource.
 */
function coap_resource_get_number_of_observers($ip, $resource){
	$query = db_select('coap_sensor_interested_user', 'users');
	$query
		->fields('users', array('uri', 'observe'))
		->condition('uri', $ip . "/" . $resource, '=')
		->condition('observe', 1, '=');
	$result = $query->execute();
	return $result->rowCount();
}

/**
 * Gets the temperature out of a payload (Celsius).
 * Returns the original payload if temperature could not be parsed.
 */
function coap_resource_get_temperature($payload){
	$pattern = '/value\s+(.*)C/i';
	if(preg_match($pattern, $payload, $matches)){
		$temperature = $matches[1];
		return $temperature;
	}
	else{
		return $payload;
	}
}

///////////////
// CALLBACKS //
///////////////

/**
 * Starts observe for the given URI.
 */
function coap_resource_handle_observing_for_uri($uri, $state){
	global $user;
	
	$uri = str_replace('|', '/', $uri); //	Put slashes back in the URI
	
	//	Get number of observers for the current URI
	$observe_result = db_select('coap_sensor_interested_user', 'users')
		->fields('users', array('uid', 'uri', 'observe'))
		->condition('uri', $uri, '=')
		->condition('device', 0, '=')
		->condition('observe', 1, '=')
		->execute();
	$num_observers = $observe_result->rowCount();
	
	//	Store in database that this user is observing or not observing the current URI
	$num_updated = db_update('coap_sensor_interested_user')
		->fields(array(
			'observe' => ($state == "start" ? 1 : 0),
		))
		->condition('uri', $uri, '=')
		->condition('uid', $user->uid, '=')
		->condition('device', 0, '=')
		->execute();
	
	//	Start observing the current URI if this user is interested and no one else was already observing
	if($state == "start" && $num_observers == 0){
		background_process_start('coap_resource_start_observing', $uri);
	}
	
	//	Get number of users that is still observing the current URI
	$observe_result = db_select('coap_sensor_interested_user', 'users')
		->fields('users', array('uid', 'uri', 'observe'))
		->condition('uri', $uri, '=')
		->condition('device', 0, '=')
		->condition('observe', 1, '=')
		->execute();
	
	//	Stop observing the URI if no one is interested anymore
	if($observe_result->rowCount() == 0){
		coap_library_stop_observing('coap_sensor', $uri);
	}
	
	//	Return value for AJAX call
	echo "<uri>" . $uri . "</uri><response>success</response>";
}

/**
 * Creates and sends the CoAP message with given method for the given URI.
 */
function coap_resource_start_request_for_uri($method, $uri, $payload=''){

	$uri = str_replace('|', '/', $uri);
	global $user;
	
	$pattern = '/([^\/]+)\/(.*)/i';
	if(preg_match($pattern, $uri, $matches)){
		//	Build message
		$coap_factory = new CoAPMessageFactory('coap_sensor', $matches[1], $matches[2]);
		switch ($method) {
			case 'GET':
				$coap_message = $coap_factory->create_basic_get_request($matches[2]);
				break;
			case 'PUT':
				$coap_message = $coap_factory->create_basic_put_request($matches[2]);
				break;
			case 'POST':
				$coap_message = $coap_factory->create_basic_post_request($matches[2]);
				break;
			case 'DELETE':
				$coap_message = $coap_factory->create_basic_delete_request($matches[2]);
				break;
		}
		if($payload != ''){
			$coap_message->set_payload(coap_library_string_to_hex_with_padding($payload));
		}
		
		$response_obj = null;
		
		if($method == "GET"){
			//	Get last value from the database for the given resource to check if value is still valid and use this value if it's still valid
			$cache_query = db_select('coap_sensor_value', 'coap_sensor_value');
			$cache_query
				->fields('coap_sensor_value', array('uri', 'uid', 'hid', 'original_response', 'max_age', 'timestamp'))
				->condition('uri', $uri, '=')
				->condition('uid', $user->uid, '=')
				->orderBy('hid', 'DESC')
				->range(0,1);
			$cache_result = $cache_query->execute();
			foreach($cache_result as $cache_record){
				//	Check if the last value has a max age and if it has, check if the value is still valid
				if($cache_record->max_age != null && time()-strtotime($cache_record->timestamp) < intval($cache_record->max_age)){
					$response_obj = $coap_factory->create_message_with_given_hex_message($cache_record->original_response);
				}
			}
		}
		
		//	Get new value if previous isn't still valid or isn't GET
		if($response_obj == null){
			$response_obj = $coap_message->send_message();
			if($response_obj != null){
				//	Insert the response into the database to add to the history of responses
				coap_resource_insert_response_into_database($response_obj, $user->uid);
			}
		}
		
		if($response_obj != null){
			print "<uri>" . $uri . "</uri><method>" . $method . "</method><response>" . $response_obj->get_payload_text() . "</response><code>" . $response_obj->get_numeric_message_code() . "</code>";
		}
	}
}

/**
 * Functie die wordt uitgevoerd als een ajax call wordt uitgevoerd naar /coap_resource/poll, geeft de laatste entry terug.
 */
function coap_resource_poll($uri){
	global $user;
	
	$uri = str_replace('|', '/', $uri); //	Put slashes back in the URI
	
	$output = "<poll><uri>" . $uri . "</uri>";
	
	//	Retrieve last error from database
	$resources_query = db_select('coap_sensor_resource', 'resources');
	$resources_query
		->fields('resources', array('uri', 'last_error'))
		->condition('uri', $uri, '=');
	$resources_result = $resources_query->execute();
	foreach($resources_result as $resources_record){
		$output .= "<error>" . $resources_record->last_error . "</error>";
	}
	
	//	Retrieve new values from database
	$query = db_select('coap_sensor_value', 'coap_values');
	$query
		->fields('coap_values', array('hid', 'uid', 'timestamp', 'max_age', 'parsed_value', 'uri'))
		->condition('uri', $uri, '=')
		->condition('uid', $user->uid, '=')
		->condition('new', 1, '=')
		->orderBy('hid', 'DESC');
	$result = $query->execute();
	$output .= "<entrys>";
	foreach ($result as $record){
		//	Store in the database that this value has been fetched and therefore not new anymore
		$hid = $record->hid;
		$num_updated = db_update('coap_sensor_value')
		->fields(array(
			'new' => 0,
		))
		->condition('hid', $hid, '=')
		->execute();
		
		//	Add the current value to the output
		$timestamp = $record->timestamp;
		$coap_value = $record->parsed_value;
		$max_age = $record->max_age;
		$output .= "<entry><hid>" . $hid . "</hid><value>" . $coap_value . "</value><max_age>" . $max_age . "</max_age><timestamp>" . $timestamp . "</timestamp></entry>";
	}
	$output .= "</entrys></poll>";
	
	//	return the output
	print $output;
}

// 	Start the observing mechanism for the resource with given URI
//	If the resource is not observable, the values will be retrieved periodically
function coap_resource_start_observing($uri){
	$pattern = '/([^\/]+)\/(.*)/i';
	if(preg_match($pattern, $uri, $matches)){
		$ip = $matches[1];
		$resource = $matches[2];
		
		$coap_factory = new CoAPMessageFactory('coap_sensor', $ip, $resource);
		
		//	Get the record from the database for given URI if it's observable, to check if the resource is observable
		$query = db_select('coap_sensor_resource', 'resources');
		$query
			->fields('resources', array('uri', 'obs'))
			->condition('obs', 1, '=')
			->condition('uri', $uri, '=');
		$result = $query->execute();
		
		if($result->rowCount() > 0){
			// resource is observable
			$message = $coap_factory->create_observe_get_request($resource);
			$message->send_message();
		}
		else{
			// resource is not observable
			$message = $coap_factory->create_basic_get_request($resource);
			$query = db_select('coap_sensor_interested_user', 'users');
			$query
				->fields('users', array('uri', 'observe'))
				->condition('observe', 1, '=')
				->condition('uri', $uri, '=');
			$result = $query->execute();
			while($result->rowCount() > 0){
				$response_obj = $message->send_message();
				coap_resource_receive_notification($response_obj);
				sleep(5); // WAARDE NOG AANPASSEN NAAR EEN GEKOZEN WAARDE DOOR DE GEBRUIKER OF DE MAX AGE
				$result = $query->execute();
			}
		}
	}
}

/**
 * Change the interval that the polling mechanism is using for this user, this is a callback for /temperatuursensor/interval/%.
 */
function coap_resource_change_interval($uri, $interval){
	global $user;
	
	$uri = str_replace('|', '/', $uri); //	Put slashes back in the URI
	
	$seconds = intval($interval);
	$id = db_update('coap_sensor_interested_user')
			->fields(array(
				'polling_interval' => $seconds,
			))
			->condition('uid', $user->uid, '=')
			->condition('uri', $uri, '=')
			->execute();
}

