<?php

/**
 * Returns form elements for core page. This is the
 * first step.
 */
function coap_discovery_form($form, &$form_state, $node) {
	drupal_add_js(drupal_get_path('module', 'coap_resource') . '/js/coap_resource.js'); // JavaScript bestand toevoegen
	drupal_add_js('https://www.google.com/jsapi'); // JavaScript bestand toevoegen
	global $user;	
	
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

/////////////////////
// submit handlers //
/////////////////////

//start de discovery-procedure en vernieuwt de waarden in de db
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
	
	// background_process_start('_start_discovery', $form_state['values']['device_uri'], $form_state['values']['nid']);
	_start_discovery($form_state['values']['device_uri'], $form_state['values']['nid']);
	
	$form_state['rebuild'] = TRUE;
}

//////////
// rest //
//////////
function coap_discovery_page_callback($nid){
	global $user;
	
	$output = '';
	$result = db_select('coap_resource_users','user_device')
		->fields('user_device', array('uid','uri','device','new'))
		->condition('uid', $user->uid, '=')
		->condition('device',1,'=')
		->condition('nid',$nid,'=')
		->execute();
	foreach($result as $record){
		$new = $record->new;
		$device_uri = $record->uri;
	}
	
	if($new == 2){
		$wrapper = entity_metadata_wrapper('node', node_load($nid));
		db_update('coap_resource_users')
			->fields(array(
				'new' => 0,
			))
			->condition('uid', $user->uid, '=')
			->condition('device',1,'=')
			->condition('nid', $nid, '=')
			->execute();
		$output .= '::new=1';
		
		//haal resources op die in core zitten: LIKE
		//voeg nieuwe entities toe en voeg een reference toe per entity
		
		$result = db_select('coap_resource_resources','resources')
			->fields('resources', array('uri'))
			->condition('uri', "$device_uri%", 'LIKE')
			->execute();
		$i=0;
		foreach($result as $record){
			$uri = $record->uri;
			$new_node = entity_create('node', array('type' => 'coap_resource'));
			$new_node->uid = $user->uid;
			$entity = entity_metadata_wrapper('node',$new_node);
			$entity->title = 'CoAP Resource: ' . $uri;
			$entity->field_resource_uri->set($uri);
			$entity->save();
			//reference toevoegen
			$wrapper->field_resource_references[$i]->set(entity_id('node', $entity)->value());
			$i++;
		} 	
		$wrapper->save();
				
	}
	print $output;
}

function coap_discovery_start_discover($device_uri, $nid){
	$coap_factory = new CoAPMessageFactory('coap_resource', $device_uri, '', false);
	$request = $coap_factory->create_basic_get_request('.well-known//core');
	$response = $request->send_message();
	$payloads = $response->get_payload();
	coap_discovery_parse_discovery_payloads($device_uri, $nid, $payloads);
}

//verwerkt de payload en voegt info over de resources toe aan db
function coap_discovery_parse_discovery_payloads($device_uri, $nid, $payloads){
	global $user;
	sleep(3);
	$payloads_string = coap_library_hex_to_string($payloads);
	$resources = array();
	$regex = '/<[^>]+>\s*(;\s*\w+\s*(=\s*(\w+|"[^"]*(\\.[^"]*)*")\s*)?)*/';
	preg_match_all($regex, $payloads_string, $parsing_output);
	
	for($i=0 ; $i < count($parsing_output[0]) ; $i++){
		if(preg_match('/^<([^>\?]+)[^>]*>\s*(;.+)?\s*$/', $parsing_output[0][$i], $elements)){
			$uri = $elements[1];
			// dpm('uri: ' . $uri);
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
