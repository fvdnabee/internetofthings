<?php

/**
 * Class which contains constants.
 */
class Constants {
	/**
	 * Contains all the supported attributes of the CoRE Link Format used for discovery.
	 */
    private static $attributes = array(
		'obs' => 1,
		'rt' => 1,
		'ct' => 1,
		'if' => 1,
		'sz' => 1,
		'title' => 1,
		'anchor' => 1,
		'rel' => 1
	);
    public static function getAttributes() {
        return self::$attributes;
    }
}

/**
 * Returns device form.
 */
function coap_device_form($form, &$form_state, $node) {
	global $user;
	
	//add JavaScript files
	drupal_add_js(drupal_get_path('module', 'coap_sensor') . '/js/coap_device.js');
	drupal_add_js('https://www.google.com/jsapi');
	
	//fetch device information of this node
	$result = db_select('coap_sensor_interested_user','device')
		->fields('device', array('uid', 'new'))
		->condition('nid', $node->nid, '=')
		->execute();
	
	//should ALWAYS be 1
	if($result->rowCount() == 1){	
		$device  = $result->fetchAssoc();
		$uid = $device['uid'];
		$new = $device['new']; //is used at the bottom
		
		//hidden field to make nid available through the form functions and submit handlers
		$form['nid'] = array
		(
			'#type' => 'hidden',
			'#value' => $node->nid,
		);	
		
		//hidden field to make device_uri available through the form functions and submit handlers
		$image = field_get_items('node', $node, 'field_device_uri');		
		$form['device_uri'] = array
		(
			'#type' => 'hidden',
			'#value' => $image[0]['value'],
		);	
		
		//button to refresh the well-know/core
		//show this button only when the user watching this form is the same as the user who added it
		if($uid == $user->uid){
			$form['refresh_device'] = array
			(
				'#type' => 'submit',
				'#value' => 'Refresh device',
				'#submit' => array('coap_device_refresh_device_core_submit'),
				'#prefix' => '</br>',
			);
		}
		
		if($new == 1 || $new == 2){
			$form['refresh_device']['#suffix'] = '<label id=lbl_message >Please wait, some changes are being loaded...</label></br>';
		}
		
		return $form;
		
		}
	else {
		throw Exception('Someone has been foefeling with the database.^^');
	}
}

/**
 * Starts the discovery procedure.
 */
function coap_device_refresh_device_core_submit($form, &$form_state) {
	global $user;
	
	db_update('coap_sensor_interested_user')
		->fields(array(
			'new' => 1,
		))
		->condition('uid', $user->uid, '=')
		->condition('uri', $form_state['values']['device_uri'], '=')
		->condition('nid', $form_state['values']['nid'], '=')
		->execute();
	
	background_process_start('_start_discovery', $form_state['values']['device_uri'], $form_state['values']['nid']);
	
	$form_state['rebuild'] = TRUE;
}

/**
 * Private function which starts a resource discovery procedure, parses the entire well-known/core and processes the information.
 */
function _start_discovery($device_uri,$nid){
	global $user;
	
	//do a resource discovery
	$coap_factory = new CoAPMessageFactory('coap_sensor', $device_uri, '', false);
	$request = $coap_factory->create_basic_get_request('.well-known/core');
	$response = $request->send_message();
	$payloads = $response->get_payload();
	$resources = coap_device_parse_discovery_payloads($device_uri, $nid, $payloads);

		
	$current_time = time();
	foreach($resources as $resource){
		$resource_uri = $resource['uri'];
		$result = db_select('coap_sensor_resource','resources')
			->fields('resources', array('uri'))
			->condition('uri', $resource_uri, '=')
			->execute();
		
		if($result->rowCount() == 0){
			$resource['created']= $current_time;
			$resource['changed']= $current_time;
			db_insert('coap_sensor_resource')			
				->fields($resource)
				->execute();
		}
		else{
			db_update('coap_sensor_resource')
				->fields(array(
					'changed' => $current_time,
					'not_in_core' => 0, //this is needed for resources that were removed from the core but are added back
				))
				->condition('uri', $resource_uri, '=')
				->execute();
		}
	}
	
	//as long as the fact that the user is interested hasn't been added to the db, wait
	$result = db_select('coap_sensor_interested_user','user_device')
		->fields('user_device', array('uid','uri','device'))
		->condition('uid', $user->uid, '=')
		->condition('device',1,'=')
		->condition('nid',$nid,'=')
		->execute();
	$counter = 0;
	while($result->rowCount() == 0 && $counter < 100){
		sleep(3); //leave this so the db doesn't get flooded
		$result = db_select('coap_sensor_interested_user','user_device')
			->fields('user_device', array('uid','uri','device'))
			->condition('uid', $user->uid, '=')
			->condition('device',1,'=')
			->condition('nid',$nid,'=')
			->execute();
		$counter++;
	}
	
	//update every entry that indicates a user being interested in a device with the given URI
	db_update('coap_sensor_interested_user')
		->fields(array(
			'new' => 2,
		))
		->condition('uri',$device_uri ,'=')
		->condition('device',1 ,'=')
		->execute();
}

/**
 * Private function which starts a partial resource discovery procedure, parses the requested information and processes it.
 */
function _start_partial_discovery($uri,$nid){
	global $user;
	$parts = explode('/',$uri);
	$device_uri = $parts[0];
	$resource_name = '/' . $parts[1]; //we already add an '/' even thought it isn't part of the resource name
	
	$coap_factory = new CoAPMessageFactory('coap_sensor', $device_uri, '', false);
	$request = $coap_factory->create_basic_get_request('.well-known/core');
	$request->add_option(15,coap_library_string_to_hex_with_padding("href=$resource_name"));
	$response = $request->send_message();
	$payloads = $response->get_payload();
	$resources = coap_device_parse_discovery_payloads($device_uri, $nid, $payloads);
	
	$resources[0]['created']= time();
	$resources[0]['changed']= time();
	db_insert('coap_sensor_resource')		
		->fields($resources[0])
		->execute();
}

/**
 * Device page callback.
 */
function coap_device_page_callback($nid){
	global $user;
	
	$output = '';
	$result = db_select('coap_sensor_interested_user','user_device')
		->fields('user_device', array('uid','uri','device','new'))
		->condition('uid', $user->uid, '=')
		->condition('device',1,'=')
		->condition('nid',$nid,'=')
		->execute();
	foreach($result as $record){
		$new = $record->new;
		$device_uri = $record->uri;
	}
	
	if($new == 1){
		$output .= 'show_counter';
	}
	
	if($new == 2){
		$wrapper = entity_metadata_wrapper('node', node_load($nid));
		db_update('coap_sensor_interested_user')
			->fields(array(
				'new' => 0,
			))
			->condition('uid', $user->uid, '=')
			->condition('device',1,'=')
			->condition('nid', $nid, '=')
			->execute();
		$output .= 'reload';
		//haal resources op die in core zitten
		//voeg nieuwe entities toe en voeg een reference toe per entity
		$result = db_select('coap_sensor_resource','resources')
			->fields('resources', array('uri','created','changed','not_in_core'))
			->condition('uri', "$device_uri%", 'LIKE')
			->orderBy('changed', 'DESC')
			->orderBy('uri', 'ASC')
			->execute();
		$i=0;
		$newest = 0;
		foreach($result as $record){
			$uri = $record->uri;
			$changed = $record->changed;
			$not_in_core = $record->not_in_core;
			if($changed >= $newest){ //is deze waarde mee geupdated met de andere waarden?
				$db_resource = db_select('coap_sensor_interested_user','user_device')
					->fields('user_device', array('uid','nid','uri','device'))
					->condition('uri', $uri, '=')
					->condition('uid', $user->uid, '=')
					->condition('device',0,'=')
					->execute();
				$rowCount = $db_resource->rowCount();
				if($rowCount != 0){
					foreach($db_resource as $record){
						$resource_nid = $record->nid;
					}
				}
				if($rowCount == 0){
					$newest = $changed;
					$new_node = entity_create('node', array('type' => 'coap_resource'));
					$new_node->uid = $user->uid;
					$entity = entity_metadata_wrapper('node',$new_node);
					$entity->title = 'CoAP Resource: ' . $uri;
					$entity->field_resource_uri->set($uri);
					$entity->save();
					//reference toevoegen
					$wrapper->field_resource_references[$i]->set(entity_id('node', $entity)->value());
				}
				else if($rowCount != 0){
					$newest = $changed;
					//zet references
					$wrapper->field_resource_references[$i]->set($resource_nid);
				}
			}
			else if($not_in_core != 1){ //zet not_in_core op 1 als het er al niet opstaat
				db_update('coap_sensor_resource')
					->fields(array(
						'not_in_core' => 1,
					))
					->condition('uri', $uri, '=')
					->execute();
				unset($wrapper->field_resource_references[$i]);
			}
			$i++;
		} 	
		$wrapper->save();
				
	}
	print $output;
}

/**
 * Parses the combined payload of a sequence of discovery requests.
 * Returns an array of resources who are represented by an array:
 * 0 =>	(	'uri' 		=> '2001:6a8:1d80:200::2/create2',
 *			'obs' 		=> '0',
 *			'rt'		=> 'core:test',
 *			'ct'		=> '0',
 *			'ifd'		=> 'core#ll',
 *			'title'		=> 'Create2 resource',
 *		)
 * 1 =>	(	'uri' 		=> '2001:6a8:1d80:200::2/large',
 *			'obs' 		=> '0',
 *			'rt'		=> 'block:block2',
 *			'ct'		=> '0',
 *			'ifd'		=> 'core#ll',
 *			'sz'		=> '1284',
 *			'title'		=> 'Large resource',
 *		)
 * 2 => (	...
 */
function coap_device_parse_discovery_payloads($device_uri, $nid, $payloads){
	global $user;
	$resources = array();
	//$payloads contains the entire payload string with every char represented by an hex value
	$payloads_string = coap_library_hex_to_string($payloads);
	$regex = '/<[^>]+>\s*(;\s*\w+\s*(=\s*(\w+|"[^"]*(\\.[^"]*)*")\s*)?)*/';
	preg_match_all($regex, $payloads_string, $parsing_output);
	
	for($i=0 ; $i < count($parsing_output[0]) ; $i++){
		if(preg_match('/^<([^>\?]+)[^>]*>\s*(;.+)?\s*$/', $parsing_output[0][$i], $elements)){
			$resource_name = $elements[1];
			if (preg_match('/[a-zA-Z]+:\/\/[^\/]+.*/', $resource_name, $bin)) {
				// absolute URI, correct
			} else {
				// fix for old Contiki implementation and others which omit the leading '/' in the link format
				if (substr($resource_name,0,1)!='/') $resource_name = '/' . $resource_name;
			}
			if($elements[2]){
				preg_match_all('/;\s*\w+\s*(=\s*(\w+|"([^"]*(\\.[^"]*)*)"))?/',$elements[2],$tokens);
				
				$resource = array();
				$resource['uri'] = $device_uri . $resource_name; //NOTE: an '/' has been added to the front of $resource_name
				for($e=0 ; $e<count($tokens[0]) ; $e++){
					preg_match('/;\s*([^<"\s;,=]+)\s*(=\s*(([^<"\s;,]+)|"([^"]*(\\.[^"]*)*)"))?/',$tokens[0][$e],$keyvalues);
					//check if the CoRE Link Format attribute if supported
					$attributes_const = Constants::getAttributes();
					if(isset($attributes_const[$keyvalues[1]])){
						if($keyvalues[1] != 'if')
							$attribute = $keyvalues[1];
						else
							$attribute = $keyvalues[1] . 'd'; //this is needed because 'if' is a reserved name and can't be used as a column name in every db
							
						$resource[$attribute] = (count($keyvalues)>2 ? ($keyvalues[4] ? $keyvalues[4] : $keyvalues[5]) : 1);
					}
				}
				array_push($resources,$resource);
			}
		}
	}
	return $resources;
}