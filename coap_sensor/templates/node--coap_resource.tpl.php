<div class = 'coap_resource' id = "<?php echo $content['field_resource_uri']['#items'][0]['value']; ?>" style="padding:5px;border:2px solid;border-radius:10px;box-shadow:10px 10px 5px #888888;" >
	<h1 class = "uri" style = "text-align:center" ><?php echo $content['field_resource_uri']['#items'][0]['value']; ?></h1>
	<div class = "header" align = "center" >
		<input type = "text" class = "POLLING_INVOER" size = "2" id = "<?php echo "polling_invoer_" . $content['field_resource_uri']['#items'][0]['value'] ?>" 
					value = "<?php 	global $user;
									$result = db_select('coap_sensor_interested_user', 'users')
									->fields('users', array('uid', 'uri', 'polling_interval'))
									//->condition('uid', $user->uid, '=')
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
		<label style = "display: inline" >Minimum: </label><input type = "text" size = "2" id = "input_minimum" style="height: 8px; font-size: 8px;" value = "NA" /> - Min data: <span id="data_minimum"></span></label> (fill in NA to use data min) - View: <span id="view_minimum"></span></br>
		<label style = "display: inline" >Maximum: </label><input type = "text" size = "2" id = "input_maximum" style="height: 8px; font-size: 8px;" value = "NA" /> - Max data: <span id="data_maximum"></span></label> (fill in NA to use data max) - View: <span id="view_maximum"></span></br>
	</div>
	<div class = "history" >
		<h1>History</h1>
		<label style = "display:inline" >Select the amount of rows to show: </label>
		<select style = "display:inline" class = 'historyselect' >
			<?php for ($i = 1; $i <= 100; $i++) { ?>
			<option class = "option<?=$i?>" value = "<?=$i?>"<?php if ($i == 5) echo ' selected="selected"';?>><?=$i?></option>
			<?php } ?>
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
						->range(0, 100)
						->orderBy('hid', 'DESC')
						->execute();
					foreach($result as $record){
						$nr_entrys++;
						$output = "<tr class = 'row" . $nr_entrys . "'";
						if($nr_entrys > 5){
							$output .= " style = 'display: none'";
						}
						$output .= " ><td>" . $record->timestamp . "</td><td class = 'coap_value' >" . $record->parsed_value . "</td></tr>\n";
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
</div>
