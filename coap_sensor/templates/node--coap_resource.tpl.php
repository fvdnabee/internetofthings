<div class = 'coap_resource' id = "<?php echo $content['field_resource_uri']['#items'][0]['value']; ?>" style="padding:5px;border:2px solid;border-radius:10px;box-shadow:10px 10px 5px #888888;" >
	<h1 class = "uri" style = "text-align:center" ><?php echo $content['field_resource_uri']['#items'][0]['value']; ?></h1>
	<div class = "header" align = "center" >
		<input type = "text" class = "POLLING_INVOER" size = "2" id = "<?php echo "polling_invoer_" . $content['field_resource_uri']['#items'][0]['value'] ?>" 
					value = "<?php 	global $user;
									$result = db_select('coap_sensor_interested_user', 'users')
									->fields('users', array('uid', 'uri', 'polling_interval'))
									->condition('uid', $user->uid, '=')
									->condition('nid', $node->nid, '=')
									->condition('uri', $content['field_resource_uri']['#items'][0]['value'], '=')
									->execute();
									foreach($result as $record){
										echo $record->polling_interval;
									} ?>" />
		<input type = "button" id = "<?php echo "btn_polling_" . $content['field_resource_uri']['#items'][0]['value'] ?>" 
					class = "POLLING_BUTTON form-submit" value = "Change polling interval" />
	</div>
	<div class = 'GET'>
		<input type = "button" id = "<?php echo "btn_GET_" . $content['field_resource_uri']['#items'][0]['value'] ?>" 
				class = "REQUEST_BUTTON form-submit" value = "GET" />
		<label id = "<?php echo 'lbl_GET_' . $content['field_resource_uri']['#items'][0]['value'] ?>"
				style="display:inline" >Perform a GET request</label>
		<img class = "img_status" src = "" style = "display: none" width = "15" height = "15" hspace = "5" />
	</div>
	<div class = 'PUT'>
		<input type = "button" id = "<?php echo "btn_PUT_" . $content['field_resource_uri']['#items'][0]['value'] ?>"
				class = "REQUEST_BUTTON form-submit" value = "PUT" />
		<input 	id = "<?php echo 'input_PUT_' . $content['field_resource_uri']['#items'][0]['value'] ?>" 
				type = "text"
				style="display:inline"
				class = "PUT_INPUT" >
		<label 	id = "<?php echo 'lbl_PUT_' . $content['field_resource_uri']['#items'][0]['value'] ?>"
				style = "display:inline" >Perform a PUT request with given value</label>
		<img class = "img_status" src = "" style = "display: none" width = "15" height = "15" hspace = "5" />
	</div>
	<div class = 'POST'>
		<input type = "button" id = "<?php echo "btn_POST_" . $content['field_resource_uri']['#items'][0]['value'] ?>"
				class = "REQUEST_BUTTON form-submit" value = "POST" />
		<input 	id = "<?php echo 'input_POST_' . $content['field_resource_uri']['#items'][0]['value'] ?>" 
				type = "text"
				style="display:inline"
				class = "POST_INPUT" >
		<label 	id = "<?php echo 'lbl_POST_' . $content['field_resource_uri']['#items'][0]['value'] ?>"
				style = "display:inline" >Perform a POST request with given value</label>
		<img class = "img_status" src = "" style = "display: none" width = "15" height = "15" hspace = "5" />
	</div>
	<div class = 'DELETE'>
		<input type = "button" id = "<?php echo "btn_DELETE_" . $content['field_resource_uri']['#items'][0]['value'] ?>"
				class = "REQUEST_BUTTON form-submit" value = "DELETE" />
		<label 	id = "<?php echo 'lbl_DELETE_' . $content['field_resource_uri']['#items'][0]['value'] ?>"
				style = "display:inline" >Perform a DELETE request</label>
		<img class = "img_status" src = "" style = "display: none" width = "15" height = "15" hspace = "5" />
	</div>
	<div class = 'OBSERVE' align = "center" >
		<input type = "button" id = "<?php echo "btn_OBSERVE_" . $content['field_resource_uri']['#items'][0]['value'] ?>"
				class = "OBSERVE_BUTTON form-submit" value = "<?php $result = db_select('coap_sensor_interested_user', 'users')
																	->fields('users', array('uid', 'uri'))
																	->condition('uid', $user->uid, '=')
																	->condition('device', 0, '=')
																	->condition('uri', $content['field_resource_uri']['#items'][0]['value'], '=')
																	->condition('observe', 1, '=')
																	->execute();
																	echo ($result->rowCount() ? 'Stop' : 'Start'); ?> Observing" />
		<img class = "img_status" src = "" style = "display: none" width = "15" height = "15" hspace = "5" />
		<label id = "<?php echo "lbl_OBSERVE_" . $content['field_resource_uri']['#items'][0]['value'] ?>" style = "text-align:center" >Observe input comes here</label>
	</div>
	<div class = "history" >
		<h1>History</h1>
		<label style = "display:inline" >Select the amount of rows to show: </label>
		<select style = "display:inline" class = 'historyselect' >
			<option class = "option1" value = "1" >1</option>
			<option class = "option2" value = "2" >2</option>
			<option class = "option3" value = "3" >3</option>
			<option class = "option4" value = "4" >4</option>
			<option class = "option5" value = "5" selected = "selected" >5</option>
			<option class = "option6" value = "6" >6</option>
			<option class = "option7" value = "7" >7</option>
			<option class = "option8" value = "8" >8</option>
			<option class = "option9" value = "9" >9</option>
			<option class = "option10" value = "10" >10</option>
		</select>
		
		<table class = "historytable" >
			<thead>
				<tr><th>Timestamp</th><th>Value</th></tr>
			</thead>
			<tbody>
				<?php
					global $user;
					$nr_entrys = 0;
					$result = db_select('coap_sensor_value', 'coap_values')
						->fields('coap_values', array('timestamp', 'parsed_value'))
						->condition('uri', $content['field_resource_uri']['#items'][0]['value'], '=')
						->condition('uid', $user->uid, '=')
						->range(0, 10)
						->orderBy('hid', 'DESC')
						->execute();
					foreach($result as $record){
						$nr_entrys++;
						$output = "<tr class = 'row" . $nr_entrys . "'";
						if($nr_entrys > 5){
							$output .= " style = 'display: none'";
						}
						$output .= " ><td>" . $record->timestamp . "</td><td class = 'coap_value' >" . $record->parsed_value . "</td></tr>";
						echo $output;
					}
					while($nr_entrys < 5){
						$nr_entrys++;
						echo "<tr class = 'row" . $nr_entrys . "' ><td></td><td class = 'coap_value' ></td></tr>";
					}
				?>
			</tbody>
		</table>
	</div>
	<div class = "graph" >
		<h1>Graph</h1>
		<label style = "display: inline" >Select a type of graph: </label>
		<select style = "display: inline" class = "graphselect" >
			<option class = "none" >None</option>
			<option class = "line" >Line</option>
			<option class = "pie" >Pie</option>
			<option class = "column" >Column</option>
		</select>
		<div id = "div_graphimage_<?php echo $content['field_resource_uri']['#items'][0]['value']; ?>" class = "graphimage" ></div>
	</div>
</div>