<?php

/**
 * Returns discovery form.
 */
function coap_discovery_form($form, &$form_state, $node) {
	drupal_add_js(drupal_get_path('module', 'coap_resource') . '/js/coap_resource.js'); // JavaScript bestand toevoegen
	drupal_add_js('https://www.google.com/jsapi'); // JavaScript bestand toevoegen
	
	global $user;
	
	
	$regex = '/^(((?=(?>.*?(::))(?!.+\3)))\3?|([\dA-F]{1,4}(\3|:(?!$)|$)|\2))(?4){5}((?4){2}|(25[0-5]|(2[0-4]|1\d|[1-9])?\d)(\.(?7)){3})\z/i';
	$value = '2001:6a8:1d80:200::1';
	
	if (empty($value) || !preg_match($regex, $value, $matches)) {
		dpm('nee');
	}
	else{
		$result = db_select('coap_resource_users','device')
			->fields('device', array('uri'))
			->condition('uri', $matches[0], '=')
			->condition('uid', $user->uid, '=')
			->execute();
		
		if($result->rowCount() != 0){
			dpm('You have already submitted this resource. Please enter another.');
		}
	}
	
	
	$form['nid'] = array
	(
		'#type' => 'hidden',
		'#value' => $node->nid,
	);	
	

	$image = field_get_items('node', $node, 'field_device_uri');		
	$form['device_uri'] = array
	(
		'#type' => 'hidden',
		'#value' => $image[0]['value'],
	);	
	
	////button to refresh the well-know/core
	$form['refresh_device'] = array
	(
		'#type' => 'submit',
		'#value' => 'Refresh device',
		'#submit' => array('coap_discovery_refresh_device_core_submit'),
	);
	
	$new = 0;
	$result = db_select('coap_resource_users','user_device')
			->fields('user_device', array('uid','uri','device','new'))
			->condition('uid', $user->uid, '=')
			->condition('device',1,'=')
			->condition('nid',$node->nid,'=')
			->execute();
	foreach($result as $record){
		$new = $record->new;
	}
	
	if($new == 1){
		$form['refresh_device']['#suffix'] = '<label>Please wait, some resources are being loaded...</label>';
	}
	
	return $form;
}

/**
 * Starts the discovery procedure.
 */
function coap_discovery_refresh_device_core_submit($form, &$form_state) {
	global $user;
	
	db_update('coap_resource_users')
		->fields(array(
			'new' => 1,
		))
		->condition('uid', $user->uid, '=')
		->condition('uri', $form_state['values']['device_uri'], '=')
		->condition('nid', $form_state['values']['nid'], '=')
		->condition('device', 1, '=')
		->execute();
	
	background_process_start('_start_discovery', $form_state['values']['device_uri'], $form_state['values']['nid']);
	//_start_discovery($form_state['values']['device_uri'], $form_state['values']['nid']);
	
	$form_state['rebuild'] = TRUE;
}

function _start_discovery($device_uri,$nid){
	global $user;
	$coap_factory = new CoAPMessageFactory('coap_resource', $device_uri, '', false);
	$request = $coap_factory->create_basic_get_request('.well-known/core');
	$response = $request->send_message();
	$payloads = $response->get_payload();
	$resources = coap_discovery_parse_discovery_payloads($device_uri, $nid, $payloads);

	$current_time = time();
	foreach($resources as $resource){
		$uri = $resource['uri'];
		$result = db_select('coap_resource_resources','resources')
			->fields('resources', array('uri'))
			->condition('uri', $uri, '=')
			->execute();
		
		if($result->rowCount() == 0){
			$resource['created']= $current_time;
			$resource['changed']= $current_time;
			db_insert('coap_resource_resources')			
				->fields($resource)
				->execute();
		}
		else{
			db_update('coap_resource_resources')
				->fields(array(
					'changed' => $current_time,
				))
				->condition('uri', $uri, '=')
				->execute();
		}
	}
	
	$result = db_select('coap_resource_users','user_device')
		->fields('user_device', array('uid','uri','device'))
		->condition('uid', $user->uid, '=')
		->condition('device',1,'=')
		->condition('nid',$nid,'=')
		->execute();
	$counter = 0;
	while($result->rowCount() == 0 && $counter < 100){
		sleep(3); //leave this so the db doesn't get flooded
		$result = db_select('coap_resource_users','user_device')
			->fields('user_device', array('uid','uri','device'))
			->condition('uid', $user->uid, '=')
			->condition('device',1,'=')
			->condition('nid',$nid,'=')
			->execute();
		$counter++;
	}
	
	db_update('coap_resource_users')
		->fields(array(
			'new' => 2,
		))
		->condition('uid',$user->uid ,'=')
		->condition('uri',$device_uri ,'=')
		->condition('nid',$nid ,'=')
		->condition('device',1 ,'=')
		->execute();
}

function _start_filtered_discovery($uri,$nid){
	global $user;
	$parts = explode('/',$uri);
	$device_uri = $parts[0];
	$resource_name = '/' . $parts[1];
	
	$coap_factory = new CoAPMessageFactory('coap_resource', $device_uri, '', false);
	$request = $coap_factory->create_basic_get_request('.well-known/core');
	$request->add_option(15,coap_library_string_to_hex_with_padding("href=$resource_name"));
	$response = $request->send_message();
	$payloads = $response->get_payload();
	$resources = coap_discovery_parse_discovery_payloads($device_uri, $nid, $payloads);
	
	// $result = db_select('coap_resource_resources','resources')
		// ->fields('resources', array('uri'))
		// ->condition('uri', $uri, '=')
		// ->execute();
	// if($result->rowCount() == 0){
		// $resources[0]['created']= time();
		// $resources[0]['changed']= time();
		// db_insert('coap_resource_resources')		
			// ->fields($resources[0])
			// ->execute();
	// }
	// else{
		// $resources[0]['created']= time();
		// $resources[0]['changed']= time();
		// unset($resources[0]['uri']);
		// db_update('coap_resource_resources')
				// ->fields($resources[0])
				// ->condition('uri', $uri, '=')
				// ->execute();
	// }
	
	$resources[0]['created']= time();
	$resources[0]['changed']= time();
	db_insert('coap_resource_resources')		
		->fields($resources[0])
		->execute();
}

/**
 * Discovery page callback.
 */
function coap_discovery_page_callback($nid){
	global $user;
	
	$output = '';
	$result = db_select('coap_resource_users','user_device')
		->fields('user_device', array('uid','uri','device','new','version_time'))
		->condition('uid', $user->uid, '=')
		->condition('device',1,'=')
		->condition('nid',$nid,'=')
		->execute();
	foreach($result as $record){
		$new = $record->new;
		$device_uri = $record->uri;
		$version_time = $record->version_time;
	}
	
	if($new == 2){
		$wrapper = entity_metadata_wrapper('node', node_load($nid));
		db_update('coap_resource_users')
			->fields(array(
				'new' => 0,
				'version_time' => time(),
			))
			->condition('uid', $user->uid, '=')
			->condition('device',1,'=')
			->condition('nid', $nid, '=')
			->execute();
		$output .= '::new=1';
		//haal resources op die in core zitten
		//voeg nieuwe entities toe en voeg een reference toe per entity
		$result = db_select('coap_resource_resources','resources')
			->fields('resources', array('uri','created','changed','deleted'))
			->condition('uri', "$device_uri%", 'LIKE')
			->execute();
		$i=0;
		foreach($result as $record){
			$uri = $record->uri;
			$db_resource = db_select('coap_resource_users','user_device')
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
			if($version_time < $record->created || $rowCount == 0){
				$new_node = entity_create('node', array('type' => 'coap_resource'));
				$new_node->uid = $user->uid;
				$entity = entity_metadata_wrapper('node',$new_node);
				$entity->title = 'CoAP Resource: ' . $uri;
				$entity->field_resource_uri->set($uri);
				$entity->save();
				//reference toevoegen
				$wrapper->field_resource_references[$i]->set(entity_id('node', $entity)->value());
			}
			else if($version_time < $record->changed || $rowCount != 0){
				//zet references
				$wrapper->field_resource_references[$i]->set($resource_nid);
			}
			//volgend stuk is nog NIET GETEST!!!!!!!!!!!
			//de waarde zit niet meer in de core, anders zou ze geupdate zijn
			else{
				//zet deleted op 0
				if($record->deleted == 0){
					db_update('coap_resource_resources')
						->fields(array(
							'deleted' => 1,
							//'changed' => time(),
						))
						->condition('uri', $uri, '=')
						->execute();
				}
				//deze resource zit er al even niet in, doe niks
				else{ 
				
				}
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
 */
function coap_discovery_parse_discovery_payloads($device_uri, $nid, $payloads){
	global $user;
	//sleep(3);
	$payloads_string = coap_library_hex_to_string($payloads);
	$resources = array();
	$regex = '/<[^>]+>\s*(;\s*\w+\s*(=\s*(\w+|"[^"]*(\\.[^"]*)*")\s*)?)*/';
	preg_match_all($regex, $payloads_string, $parsing_output);
	
	for($i=0 ; $i < count($parsing_output[0]) ; $i++){
		if(preg_match('/^<([^>\?]+)[^>]*>\s*(;.+)?\s*$/', $parsing_output[0][$i], $elements)){
			$uri = $elements[1];
			if (preg_match('/[a-zA-Z]+:\/\/[^\/]+.*/', $uri, $bin)) {
				// absolute URI, correct
			} else {
				// fix for old Contiki implementation and others which omit the leading '/' in the link format
				if (substr($uri,0,1)!='/') $uri = '/' . $uri;
			}
			if($elements[2]){
				preg_match_all('/;\s*\w+\s*(=\s*(\w+|"([^"]*(\\.[^"]*)*)"))?/',$elements[2],$tokens);
				
				$resource = array();
				$resource['uri'] = $device_uri . $uri;
				for($e=0 ; $e<count($tokens[0]) ; $e++){
					preg_match('/;\s*([^<"\s;,=]+)\s*(=\s*(([^<"\s;,]+)|"([^"]*(\\.[^"]*)*)"))?/',$tokens[0][$e],$keyvalues);
					if($keyvalues[1] != 'if') $attribute = $keyvalues[1];
					else $attribute = $keyvalues[1] . 'd';
					$attributes[$attribute] = (count($keyvalues)>2 ? ($keyvalues[4] ? $keyvalues[4] : $keyvalues[5]) : 1);
					
					$resource[$attribute] = $attributes[$attribute];
				}
				array_push($resources,$resource);
			}
		}
	}
	return $resources;
}