<?php

function coap_discovery_build_headers(){
return array
  (
  
    // 'uri' => 'uri',	
	// 'rt' => 'application-specific semantic type',
	// 'ct' => 'content-type',
	// 'if' => 'interface desciption',
	// 'sz' => 'maximum size',
	// 'title' => 'human-readable name',
	// 'anchor' => 'anchor',
	// 'rel' => 'rel',
	
	'uri' => 'uri',
	'title' => 'human-readable name',
  );
}

function coap_discovery_build_options($resources){
  $options = array();
  $counter=1;
  foreach($resources as $resource)
  {
    //$options[$resource['uid']] = array // each element of the array is keyed with the UID
	$options[$counter] = array // each element of the array is keyed with the UID
    (
	
		// 'uri' => $resource['uri'],
		// 'rt' => $resource['rt'],
		// 'ct' => $resource['ct'],
		// 'if' => $resource['if'],
		// 'sz' => $resource['sz'],
		// 'title' => $resource['title'],
		// 'anchor' => $resource['anchor'],
		// 'rel' => $resource['rel'],	
		
		'uri' => $resource['uri'],
		'title' => $resource['title'],
	);
	$counter++;
  }
  return $options;
}

/**
 * Custom validation form for the core page.
 */
function coap_discovery_form_validate($form, &$form_state) {
	$regex = '/^(((?=(?>.*?(::))(?!.+\3)))\3?|([\dA-F]{1,4}(\3|:(?!$)|$)|\2))(?4){5}((?4){2}|(25[0-5]|(2[0-4]|1\d|[1-9])?\d)(\.(?7)){3})\z/i';
	if(!preg_match($regex, $form_state['values']['device_uri'], $matches)){
		form_set_error('device_uri','Please enter a valid IPv6-adres.');
	}
}

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
	
	if($node->type == 'coap_general'){
		//textfield for device uri
		$form['device_uri'] = array
		(
			'#type' => 'textfield',
			'#title' => 'Device IPv6 adress',
			'#size' => 60,
			'#required' => TRUE,
			//default value wordt later toegevoegd als uid niet nul is
		);	
		if($user->uid !=0){
		//button to load a cached well-know/core
			$form['load_device'] = array
			(
				'#type' => 'submit',
				'#value' => 'Load device',
				'#submit' => array('coap_discovery_load_device_core_submit'),
			);
		}
	}
	else{
		//$node = node_load($nid);
		$image = field_get_items('node', $node, 'field_device_uri');
		// $output = field_view_value('node', $node, 'field_device_uri', $image[0]);

		
		$form['device_uri'] = array
		(
			'#type' => 'hidden',
			'#value' => $image[0]['value'],
		);	
	}
	
	//button to refresh the well-know/core
	$form['refresh_device'] = array
	(
		'#type' => 'submit',
		'#value' => 'Refresh device',
		'#submit' => array('coap_discovery_refresh_device_core_submit'),
	);
		
	if($node->type == 'coap_general'){
		//button to clear tableselect
		$form['clear_tableselect'] = array
		(
			'#type' => 'submit',
			'#value' => 'Clear',
			'#attributes' => array('id' => 'core_table'),
			'#submit' => array('coap_discovery_clear_tableselect_submit'),
		);	
	}
	
	$form['tableselect_container'] = array(
		'#type' => 'container',
		'#attributes' => array(
			'class' => array('class-name'),
		),
	);
	
	if($user->uid != 0){ //geen anonieme user	
		$_SESSION[$user->sid]['resources'] = array();	
		if($user->uid != 0){ //geen anonieme user
			$result = db_select('coap_resource_users','user_device')
				->fields('user_device', array('uid','uri','device'))
				->condition('uid', $user->uid, '=')
				->condition('device',1,'=')
				->condition('nid',$node->nid,'=')
				->execute();
		}
		foreach($result as $record){
			$device_uri = $record->uri;
			$form['device_uri']['#default_value'] = $device_uri;	//alleen nutig als type = coap_general	
			$db_resources= db_select('coap_resource_resources', 'resources')
				->fields('resources', array('uri','title'))
				->condition('uri', "$device_uri%", 'LIKE')
				->execute();
			$resources = array();
			foreach($db_resources as $record){
				$resource = array();
				$resource['uri'] = $record->uri;
				$resource['title'] = $record->title;
				array_push($resources,$resource);
			} 		
			$_SESSION[$user->sid]['resources'] = $resources;
		}
	}
	else if(isset($_SESSION[$user->sid])){
		$form['device_uri']['#default_value'] = $_SESSION[$user->sid]['device_uri'];
	}
	
	// if($user->uid != 0){ //geen anonieme user
		// $result = db_select('coap_resource_users','user_device')
				// ->fields('user_device', array('uid','uri','device'))
				// ->condition('uid', $user->uid, '=')
				// ->condition('device',1,'=')
				// ->execute();
	// }
	
	$resources = array();
	
	//build headers
	$header = coap_discovery_build_headers();
	
	//build options
	$options = coap_discovery_build_options($resources);
	
	$table = array
	(
		'#type' => 'tableselect',
		'#header' => $header,
		'#options' => $options,
		'#multiple' => TRUE,
	);
	
	
	if(($user->uid != 0 && isset($result) && $result->rowCount() != 0) || (isset($_SESSION) && $user->uid == 0 && $_SESSION[$user->sid]['resources']))
		$table['#empty'] = 'Fetching sensors...';
	else if($node->type == 'coap_general')
		$table['#empty'] = 'Enter a URI and click on \'Load device\' or \'Refresh device\' to display resources here.';
	else	
		$table['#empty'] = 'Click \'Refresh device\'.';
		
	$form['tableselect_container']['table']= $table;
	
	$form['save'] = array(
		'#type' => 'submit',
		'#value' => 'Save',
		'#submit' => array('coap_discovery_save_resources'),
	);
	
	return $form;
}

/////////////////////
// submit handlers //
/////////////////////

function coap_discovery_load_device_core_submit($form, &$form_state) {
	//dpm('refresh submit');
	//background_process_start('coap_discovery_load_device_core', $form_state['values']['device_uri'], $form_state['values']['nid']);  
	coap_discovery_load_device_core($form_state['values']['device_uri'],$form_state['values']['nid']); //voorgrond
	$form_state['rebuild'] = TRUE;
}

//start de discovery-procedure en voegt toe aan de db dat de user geïnteresseerd is in dit device
function coap_discovery_refresh_device_core_submit($form, &$form_state) {
	global $user;
	//dpm($form_state['values']['device_uri']);
	//background_process_start('coap_discovery_start_discover', $form_state['values']['device_uri', $form_state['values']['nid']]);
	coap_discovery_start_discover($form_state['values']['device_uri'],$form_state['values']['nid']); //voorgrond
	
	$result = db_select('coap_resource_users', 'coap_user')
			->fields('coap_user', array('uri','nid','device','uid'))
			->condition('uid', $user->uid, '=')
			->condition('uri', $form_state['values']['device_uri'], '=')
			->condition('nid', $form_state['values']['nid'],'=')
			->condition('device', 1, '=')
			->execute();
		
	if($result->rowCount() == 0 && $user->uid != 0){ //tweede stuk is eigenlijk overbodig, maar je weet nooit
		$id = db_insert('coap_resource_users')
			->fields(array(
				'uid' => $user->uid,
				'uri' => $form_state['values']['device_uri'],				
				'nid' => $form_state['values']['nid'],
				'device' => 1,
				))
			->execute();
	}
	
	$form_state['rebuild'] = TRUE;
}

function coap_discovery_clear_tableselect_submit($form, &$form_state) {
	global $user;
	if($user->uid!=0){
		$_SESSION[$user->sid]['resources'] = array();
		db_delete('coap_resource_users')
			->condition('uid', $user->uid, '=')
			->condition('device',1,'=')
			->execute();
			
	}
	else $_SESSION[$user->sid]['resources'] = array();
	$form_state['rebuild'] = TRUE;
}

function coap_discovery_save_resources($form, &$form_state) {
	global $user;
	$nid = $form_state['values']['nid'];
	foreach($form_state['values']['table'] as $value){
		$resource = $_SESSION[$user->sid]['resources'][$value];
		$uri = $resource['uri'];
		
		// Watch bij alle rijen van de user op nul zetten
		$num_updated = db_update('coap_resource_users')
			->fields(array(
				'watch' => 0,
			))
			->condition('uid', $user->uid, '=')
			->condition('nid', $nid, '=')
			->execute();
		
		// Watch van net toegevoegde URI op 1 zetten
		$result = db_select('coap_resource_users', 'coap_resource')
			->fields('coap_resource', array('uri'))
			->condition('uri', $uri, '=')
			->condition('nid',$nid,'=')
			->execute();
		
		if($result->rowCount() == 0){
			$id = db_insert('coap_resource_users')
				->fields(array(
					'uid' => $user->uid,
					'uri' => $uri,
					'observe' => 0,
					'watch' => 1,
					'responded' => 0,
					'nid' => $nid,
				))
				->execute();
		}
		else{
			$num_updated = db_update('coap_resource_users')
				->fields(array(
					'watch' => 1,
				))
				->condition('uid', $user->uid, '=')
				->execute();
		}
	}
	
	//$form_state['values']['resource'] = $resource;
	//dpm(substr($form['#action'],1) . '/view/resources');
	$form_state['redirect'] = substr($form['#action'],1) . '/view/resources';
	//$form_state['redirect'] = substr($form['#action'],1,-10);
	//$form_state['rebuild'] = TRUE;  // Force rebuild with next step.
	return;
}

//////////
// rest //
//////////
function coap_discovery_page_callback($nid){
	global $user;
	$output ='';
	
	//if($user->uid != 0 && !isset($_SESSION[$user->sid]['resources'])){
	if($user->uid != 0){
		$db_device_uri = db_select('coap_resource_users','user_device')
				->fields('user_device', array('uid','uri','device'))
				->condition('uid', $user->uid, '=')
				->condition('device',1,'=')
				->condition('nid',$nid,'=')
				->execute();
		foreach($db_device_uri as $record){
			$device_uri = $record->uri;
		}
		
		if($db_device_uri->rowCount() != 0){
			$db_user_resources = db_select('coap_resource_users', 'users')
					->fields('users', array('uri','uid','device'))
					->condition('uri', "$device_uri%", 'LIKE')
					->condition('uid', $user->uid, '=')
					->condition('device', 0, '=')
					->condition('nid',$nid,'=')
					->execute();
			$resource_hash = array();
			foreach($db_user_resources as $record){
				$resource_hash[$record->uri] = 1;
			}
			$db_resources = db_select('coap_resource_resources', 'resources')
					->fields('resources', array('uri','title'))
					->condition('uri', "$device_uri%", 'LIKE')
					->execute();
			foreach($db_resources as $record){
				$output .= '<resource>';
				$output .= '<uri>';
				$output .= $record->uri;
				$output .= '</uri>';
				$output .= '<title>';
				$output .= $record->title;
				$output .= '</title>';
				$output .= '<watch>';
				$output .= isset($resource_hash[$record->uri]) ? 1 : 0;
				//output .= 1;
				$output .= '</watch>';
				$output .='</resource>';
			}
			if($db_resources->rowCount() == 0) $output = 'No data in database about this device. Click on \'Refresh device\' to display resources.';
		}
	}	
	else{
		for($i=0 ; $i<count($_SESSION[$user->sid]['resources']) ; $i++){
			$output .= '<resource>';
			$output .= '<uri>';
			$output .= $_SESSION[$user->sid]['resources'][$i]['uri'];
			$output .= '</uri>';
			$output .= '<title>';
			$output .= $_SESSION[$user->sid]['resources'][$i]['title'];
			$output .= '</title>';
			$output .= '<watch>';
			$output .= $_SESSION[$user->sid]['resources'][$i]['title'];
			$output .= '</watch>';
			$output .='</resource>';
		}
	}
	
	print $output;
}

function coap_discovery_load_device_core($device_uri, $nid){
	//dpm($nid);
	global $user;
	// $result = db_select('coap_resource_resources', 'resources')
				// ->fields('resources', array('uri','title'))
				// ->condition('uri', "$device_uri%", 'LIKE')
				// ->execute();
	
	if($user->uid != 0){ //db aanpassen
		db_delete('coap_resource_users')
				->condition('uid', $user->uid, '=')
				->condition('device',1,'=')
				->condition('nid',$nid,'=')
				->execute();
				
		//if($result->rowCount() != 0){
			db_insert('coap_resource_users')
					->fields(array(
						'uid' => $user->uid,
						'uri' => $device_uri,
						'device' => 1,
						'nid' => $nid,
						))
					->execute();
		//}
	}
	$_SESSION[$user->sid]['device_uri'] = $device_uri;
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
	$payloads_string = coap_library_hex_to_string($payloads);
	global $user;
	$resources = array();
	$regex = '/<[^>]+>\s*(;\s*\w+\s*(=\s*(\w+|"[^"]*(\\.[^"]*)*")\s*)?)*/';
	preg_match_all($regex, $payloads_string, $resources);
	
	if($user->uid != 0){
		db_delete('coap_resource_users')
				->condition('uid', $user->uid, '=')
				->condition('device',1,'=')
				->condition('nid',$nid,'=')
				->execute();
		
		$query = db_delete('coap_resource_resources')
			->condition('uri', "$device_uri%", 'LIKE')
			->execute();
	}
	$id=-1;
	for($i=0 ; $i < count($resources[0]) ; $i++){
		if(preg_match('/^<([^>\?]+)[^>]*>\s*(;.+)?\s*$/', $resources[0][$i], $elements)){ //tweede nul moet $i zijn
			$uri = $elements[1];
			if (preg_match('/[a-zA-Z]+:\/\/[^\/]+.*/', $uri, $bin)) {
				// absolute URI, correct
			} else {
				// fix for old Contiki implementation and others which omit the leading '/' in the link format
				if (substr($uri,0,1)!='/') $uri = '/' . $uri;
			}
			if($elements[2]){
				preg_match_all('/;\s*\w+\s*(=\s*(\w+|"([^"]*(\\.[^"]*)*)"))?/',$elements[2],$tokens);
				
				$attributes = array(
					'uri' => $device_uri . $uri,
				);
				$resource = array();
				$resource['uri'] = $uri;
				for($e=0 ; $e<count($tokens[0]) ; $e++){
					preg_match('/;\s*([^<"\s;,=]+)\s*(=\s*(([^<"\s;,]+)|"([^"]*(\\.[^"]*)*)"))?/',$tokens[0][$e],$keyvalues);
					if($keyvalues[1] != 'if') $attribute = $keyvalues[1];
					else $attribute = $keyvalues[1] . 'd';
					$attributes[$attribute] = (count($keyvalues)>2 ? ($keyvalues[4] ? $keyvalues[4] : $keyvalues[5]) : 1);
					
					$resource[$attribute] = $attributes[$attribute];
				}
				array_push($resources,$resource);
			}
			
			if($user->uid != 0){			
				$id = db_insert('coap_resource_resources')
				->fields($attributes)
				->execute();
			}
		}
		else{
			//variable_set("response_string_$i", 'no match');
		}		
	}
	if($user->uid == 0) $_SESSION[$user->sid]['resources'] = $resources;
}
