<div class = 'coap_resource' style="padding:5px;border:2px solid;border-radius:10px;box-shadow:10px 10px 5px #888888;" >
	<h1 class = "uri" style = "text-align:center" ><?php echo $content['field_resource_uri']['#items'][0]['uri']; ?></h1>
	<div align = "center" >
		<input type = "text" class = "POLLING_INVOER" size = "2" id = "<?php echo "polling_invoer_" . $content['field_resource_uri']['#items'][0]['uri'] ?>" 
					value = "<?php 	global $user;
									$result = db_select('coap_resource_users', 'users')
									->fields('users', array('uid', 'uri', 'polling_interval'))
									->condition('uid', $user->uid, '=')
									->condition('nid', $node->nid, '=')
									->condition('uri', $content['field_resource_uri']['#items'][0]['uri'], '=')
									->execute();
									foreach($result as $record){
										echo $record->polling_interval;
									} ?>" />
		<input type = "button" id = "<?php echo "btn_polling_" . $content['field_resource_uri']['#items'][0]['uri'] ?>" 
					class = "POLLING_BUTTON form-submit" value = "Change polling interval" />
	</div>
	<div class = 'GET'>
		<input type = "button" id = "<?php echo "btn_GET_" . $content['field_resource_uri']['#items'][0]['uri'] ?>" 
				class = "REQUEST_BUTTON form-submit" value = "GET" />
		<label id = "<?php echo 'lbl_GET_' . $content['field_resource_uri']['#items'][0]['uri'] ?>"
				style="display:inline" >Perform a GET request</label>
	</div>
	<div class = 'PUT'>
		<input type = "button" id = "<?php echo "btn_PUT_" . $content['field_resource_uri']['#items'][0]['uri'] ?>"
				class = "REQUEST_BUTTON form-submit" value = "PUT" />
		<input 	id = "<?php echo 'input_PUT_' . $content['field_resource_uri']['#items'][0]['uri'] ?>" 
				type = "text"
				style="display:inline"
				class = "PUT_INPUT" >
		<label 	id = "<?php echo 'lbl_PUT_' . $content['field_resource_uri']['#items'][0]['uri'] ?>"
				style = "display:inline" >Perform a PUT request with given value</label>
	</div>
	<div class = 'POST'>
		<input type = "button" id = "<?php echo "btn_POST_" . $content['field_resource_uri']['#items'][0]['uri'] ?>"
				class = "REQUEST_BUTTON form-submit" value = "POST" />
		<input 	id = "<?php echo 'input_POST_' . $content['field_resource_uri']['#items'][0]['uri'] ?>" 
				type = "text"
				style="display:inline"
				class = "POST_INPUT" >
		<label 	id = "<?php echo 'lbl_POST_' . $content['field_resource_uri']['#items'][0]['uri'] ?>"
				style = "display:inline" >Perform a POST request with given value</label>
	</div>
	<div class = 'DELETE'>
		<input type = "button" id = "<?php echo "btn_DELETE_" . $content['field_resource_uri']['#items'][0]['uri'] ?>"
				class = "REQUEST_BUTTON form-submit" value = "DELETE" />
		<label 	id = "<?php echo 'lbl_DELETE_' . $content['field_resource_uri']['#items'][0]['uri'] ?>"
				style = "display:inline" >Perform a DELETE request</label>
	</div>
	<div class = 'OBSERVE' style = "text-align:center" >
		<input type = "button" id = "<?php echo "btn_OBSERVE_" . $content['field_resource_uri']['#items'][0]['uri'] ?>"
				class = "OBSERVE_BUTTON form-submit" value = "<?php $result = db_select('coap_resource_users', 'users')
																	->fields('users', array('uid', 'uri'))
																	->condition('uid', $user->uid, '=')
																	->condition('device', 0, '=')
																	->condition('uri', $content['field_resource_uri']['#items'][0]['uri'], '=')
																	->condition('observe', 1, '=')
																	->execute();
																	echo ($result->rowCount() ? 'Stop' : 'Start'); ?> Observing" />
		<label id = "<?php echo "lbl_OBSERVE_" . $content['field_resource_uri']['#items'][0]['uri'] ?>" style = "text-align:center" >Observe input comes here</label>
	</div>
</div>
<div style = "visibility:hidden" >
	<?php echo render($content); ?>
</div>