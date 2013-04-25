<?php


////////////////////////////////
// CoAP Message Factory Class //
////////////////////////////////

class CoAPMessage {
	
    private $message_hex = "";
	private $module = null;
	private $ip;
	private $resource;
	private $separate_response;
	private $format_mapping;
	private $response_code_mapping;
	private $retransmit_counter = 0;
	
	// CONSTANT
	private $ACK_TIMEOUT = 2000; // 2 seconds
	private $MAX_RETRANSMIT = 4;
	private $RANDOM_FACTOR = 1.5;
	private $NSTART = 1;
	private $DEFAULT_LEISURE = 5000; // 5 seconds
	private $PROBING_RATE = 5; // 1 Byte/second
	
	// Constructor with optional given message, expects hexadecimal message
	public function __construct($module, $ip, $resource, $separate_response = false, $message = ''){
        $this->message_hex = $message;
		$this->module = $module;
		$this->ip = $ip;
		$this->resource = $resource;
		$this->separate_response = $separate_response;
		$this->response_code_mapping = array(
			0 => 'Empty',
			65 => '2.01 Created',
			66 => '2.02 Deleted',
			67 => '2.03 Valid',
			68 => '2.04 Changed',
			69 => '2.05 Content',
			128 => '4.00 Bad Request',
			129 => '4.01 Unauthorized',
			130 => '4.02 Bad Option',
			131 => '4.03 Forbidden',
			132 => '4.04 Not Found',
			133 => '4.05 Method Not Allowed',
			134 => '4.06 Not Acceptable',
			140 => '4.12 Precondition Failed',
			141 => '4.13 Request Entity Too Large',
			143 => '4.15 Unsupported Content-Format',
			160 => '5.00 Internal Server Error',
			161 => '5.01 Not Implemented',
			162 => '5.02 Bad Gateway',
			163 => '5.03 Service Unavailable',
			164 => '5.04 Gateway Timeout',
			165 => '5.05 Proxying Not Supported'
		);
		$this->format_mapping = array(
			0 => 'text/plain',
			40 => 'application/link-format',
			41 => 'application/xml',
			42 => 'application/octet-stream',
			47 => 'application/exi',
			50 => 'application/json'
		);
    }
	
	// Getter for the hexadecimal message
	public function getHexMessage(){
		return $this->message_hex;
	}
	
	// Setter for the hexadecimal message
	public function setHexMessage($newmessage){
		$this->message_hex = $newmessage;
	}
	
	// Getter for the message (ASCII)
	public function getMessage(){
		return coap_library_hex_to_string($this->message_hex);
	}
	
	// Setter for the message (ASCII)
	public function setMessage($newmessage){
		$this->message_hex = coap_library_string_to_hex_with_padding($newmessage);
	}
	
	// Getter for the ip of the device
	public function get_ip(){
		return $this->ip;
	}
	
	// Getter for the uri path of the resource
	public function get_resource(){
		return $this->resource;
	}
	
	// Adds an option with given number and value to the given message
	// Returns null if message is smaller than 4 bytes
	public function add_option($option_number, $value){
		
		if(strlen($this->message_hex) < 8){
			return null;
		}
		$token_length = hexdec(substr($this->message_hex, 1, 1));
		if(strlen($this->message_hex) < 8+($token_length*2)){
			return null;
		}
		$pos = 8 + ($token_length*2);
		$current_number = 0;
		$option_delta = hexdec(substr($this->message_hex, $pos, 1));
		while($pos < strlen($this->message_hex) && $option_number >= $current_number + $option_delta){
			$extra = 0;
			if($option_delta == 13){
				$option_delta = hexdec(substr($this->message_hex, $pos+2, 2))-13;
				$extra += 2;
			}
			else if($option_delta == 14){
				$option_delta = hexdec(substr($this->message_hex, $pos+2, 4))-269;
				$extra += 4;
			}
			$option_length = hexdec(substr($this->message_hex, $pos+1, 1));
			if($option_length == 13){
				$option_length = hexdec(substr($this->message_hex, $pos+2+$extra, 2))-13;
				$extra += 2;
			}
			else if($option_length == 14){
				$option_length = hexdec(substr($this->message_hex, $pos+2+$extra, 4))-269;
				$extra += 4;
			}
			$option_length *= 2;
			$current_number += $option_delta;
			$pos += 2 + $extra + $option_length;
			if($pos < strlen($this->message_hex)){
				$option_delta = hexdec(substr($this->message_hex, $pos, 1));
			}
		}
		$value_length = strlen($value);
		$mid = null;
		// Add Option Delta
		if($option_number-$current_number > 268){
			$mid .= "e";
		}
		else if($option_number-$current_number > 13){
			$mid .= "d";
		}
		else{
			$mid .= dechex($option_number-$current_number);
		}
		
		// Add Option Length
		if($value_length > 268){
			$mid .= "e";
		}
		else if($value_length > 13){
			$mid .= "d";
		}
		else{
			$mid .= dechex($value_length);
		}
		
		// Add optional extended Option Delta
		if($option_number-$current_number > 268){
			$temp = coap_library_string_to_hex_with_padding($option_number-269-$current_number);
			str_pad($temp, 4, "0", STR_PAD_LEFT);
			$mid .= $temp;
		}
		else if($option_number-$current_number > 12){
			$temp = coap_library_string_to_hex_with_padding($option_number-13-$current_number);
			str_pad($temp, 2, "0", STR_PAD_LEFT);
			$mid .= $temp;
		}
		
		// Add optional extended Option Length
		if($value_length > 268){
			$temp = coap_library_string_to_hex_with_padding($value_length-269);
			str_pad($temp, 4, "0", STR_PAD_LEFT);
			$mid .= $temp;
		}
		else if($value_length > 12){
			$temp = coap_library_string_to_hex_with_padding($value_length-13);
			str_pad($temp, 2, "0", STR_PAD_LEFT);
			$mid .= $temp;
		}
		
		// Add Option Value
		$mid .= coap_library_string_to_hex_with_padding($value);
		
		if($pos >= strlen($this->message_hex)){
			$this->message_hex .= $mid;
		}else{
			$start = substr($this->message_hex, 0, $pos);
			$end = substr($this->message_hex, $pos);
			$this->message_hex = $start . $mid . $end; // nog rekening houden met eventueel extra option delta veld en extra option length veld
			//variable_set('response_string_0',$start . ',,,' . $mid . ',,,' . $end);
		}
	}
	
	// Adds a payload to the given message, if not yet present
	public function add_payload($payload){
		if(strlen($this->message_hex) < 8){
			return null;
		}
		if(strpos($this->message_hex, 'ff', 0)){
			return null;
		}
		$this->message_hex .= 'ff' . coap_library_string_to_hex_with_padding($payload);
	}
	
	// Gets the value of the option with given number
	// Returns -1 if the option with given number is not present
	public function get_option_value($option_number){
		$follow_up = $this->message_hex . "\r\n";
		if(strlen($this->message_hex) < 8){
			return -1;
		}
		$token_length = hexdec(substr($this->message_hex, 1, 1));
		if(strlen($this->message_hex) < 8+($token_length*2)){
			return -1;
		}
		$pos = 8 + ($token_length*2);
		$current_number = 0;
		if($pos < strlen($this->message_hex)){
			$extra = 0;
			$option_delta = hexdec(substr($this->message_hex, $pos, 1));
			if($option_delta == 13){
				$option_delta = hexdec(substr($this->message_hex, $pos+2, 2))-13;
				$extra += 2;
			}
			else if($option_delta == 14){
				$option_delta = hexdec(substr($this->message_hex, $pos+2, 4))-269;
				$extra += 4;
			}
			$option_length = hexdec(substr($this->message_hex, $pos+1, 1));
			if($option_length == 13){
				$option_length = hexdec(substr($this->message_hex, $pos+2+$extra, 2))-13;
				$extra += 2;
			}
			else if($option_length == 14){
				$option_length = hexdec(substr($this->message_hex, $pos+2+$extra, 4))-269;
				$extra += 4;
			}
			$option_length *= 2;
		}
		while($pos < strlen($this->message_hex) && $option_number > $current_number + $option_delta){
			$follow_up .= "nr: " . ($current_number + $option_delta) . ", length: " . $option_length . "; ";
			$current_number += $option_delta;
			$pos += 2 + $extra + $option_length;
			if($pos < strlen($this->message_hex)){
				$extra = 0;
				$option_delta = hexdec(substr($this->message_hex, $pos, 1));
				if($option_delta == 13){
					$option_delta = hexdec(substr($this->message_hex, $pos+2, 2))-13;
					$extra += 2;
				}
				else if($option_delta == 14){
					$option_delta = hexdec(substr($this->message_hex, $pos+2, 4))-269;
					$extra += 4;
				}
				$option_length = hexdec(substr($this->message_hex, $pos+1, 1));
				if($option_length == 13){
					$option_length = hexdec(substr($this->message_hex, $pos+2+$extra, 2))-13;
					$extra += 2;
				}
				else if($option_length == 14){
					$option_length = hexdec(substr($this->message_hex, $pos+2+$extra, 4))-269;
					$extra += 4;
				}
				$option_length *= 2;
			}
		}
		$follow_up .= "nr: " . ($current_number + $option_delta) . ", length: " . $option_length;
		if($pos > strlen($this->message_hex) || $option_number != $current_number + $option_delta){
			return -1;
		}
		return substr($this->message_hex, $pos+2+$extra, $option_length);
	}
	
	// Retrieves the max-age value, if present, from a message
	// Returns null if no max age option is included
	public function get_max_age(){
		$max_age_hex = $this->get_option_value(14);
		if($max_age_hex == -1){
			return 0;
		}
		return hexdec($max_age_hex);
	}
	
	// Retrieves the payload, if present, from a message
	// Returns null if no payload is present
	public function get_payload(){
		if(strlen($this->message_hex) < 8){
			return null;
		}
		$pos = strrpos($this->message_hex, 'ff', 0);
		if(!$pos){
			return null;
		}
		return coap_library_hex_to_string(substr($this->message_hex, $pos+2));
	}
	
	// 	Returns a response object with the response if the request doesn't have an observe option
	//	Returns null if the request holds an observe option
	//	Returns null if something went wrong
	//	Calls hook_receive_notification for notifications that have arrived from an observable resource
	//	Calls hook_receive_notification if something went wrong
	public function send_message(){
		//	Determine if request has an observe option
		$observe = ($this->get_option_value(6) == -1) ? 0 : 1;
		
		$request_token = $this->get_token();
		$request_message_id = $this->get_message_id();
		
		if($observe){
			//	Request has an observe option
			//	Check if resource is already being observed
			$query = db_select('coap_library_observe_resources', 'resources');
			$query
				->fields('resources', array('module', 'uri'))
				->condition('uri', $this->ip . '/' . $this->resource, '=')
				->condition('module', $this->module, '=');
			$result = $query->execute();
			//	function can be aborted if resource is already being observed
			if($result->rowCount() != 0){
				return null;
			}
			
			//	Add resource to database table that holds which resources are being observed
			db_insert('coap_library_observe_resources')
				->fields(array(
					'uri' => $this->ip . '/' . $this->resource,
					'module' => $this->module,
				))
				->execute();
			$must_observe = 1;
		}
		
		$response = 'empty';
		$port = 5683;
		$socket = pfsockopen("udp://[" . $this->ip . "]", $port, $errno, $errstr); // Open UDP-socket
		if($socket){
			//Socket opened
			//	notify using module that there are no difficulties at the moment
			$function_name = $this->module . '_receive_error';
			$function_name('none', $this->ip, $this->resource);
			
			//	Set initial timeout
			$timeout = rand($this->ACK_TIMEOUT, $this->ACK_TIMEOUT*$this->RANDOM_FACTOR); 
			stream_set_timeout($socket, 0, $timeout*1000); // stream_set_timeout expects third parameter to be in MICROseconds
			$this->retransmit_counter = 0;
			
			//	Send CoAP packet
			$message = pack('H*', $this->message_hex);
			$returnvalue = fwrite($socket, $message); //	Write to socket
			if($this->is_confirmable()){
				//	Message type = CON
				if($returnvalue != FALSE){
					$response = fread($socket, 512); //	Read from UDP-socket
				}
				
				//	Keep resending the packet and multiplying the timeout if something went wrong (exponential backoff)
				while($this->retransmit_counter < $this->MAX_RETRANSMIT && ($response == '' || $response == 'empty')){
					//	Notify the using module that something went wrong
					$function_name = $this->module . '_receive_error';
					$function_name('delay', $this->ip, $this->resource);
					
					//Set new timeout
					$this->retransmit_counter++;
					$timeout *= 2;
					stream_set_timeout($socket, 0, $timeout*1000);
					
					//	Resend the CoAP packet
					$returnvalue = fwrite($socket, $message); //	Write to socket
					if($returnvalue != FALSE){
						$response = fread($socket, 512); //	Read from socket
					}
				}
			}
			
			if($response != '' && $response != 'empty'){
				$response_obj = new CoAPMessage($this->module, $this->ip, $this->resource, $this->separate_response, coap_library_string_to_hex_with_padding($response)); //	Put response in CoAPMessage-object
				
				//	notify using module that there are no difficulties at the moment
				$function_name = $this->module . '_receive_error';
				$function_name('none', $this->ip, $this->resource);
				
				//	Reset timeout
				$this->retransmit_counter = 0;
				$timeout = rand($this->ACK_TIMEOUT, $this->ACK_TIMEOUT*$this->RANDOM_FACTOR);
				stream_set_timeout($socket, 0, $timeout*1000);
				
				if($observe){
					//	Resource should be observed
					$must_observe = $this->observe_is_necessary(); //	Check if resource should still be observed
					
					//	Set initial values for reordering
					$V1 = 0;
					$T1 = time();
					
					//	Keep receiving notifications while observe is still necessary
					while($must_observe > 0 && $response != '' && $response != 'empty'){
						//	Resource should still be observed
						
						$response_hex = coap_library_string_to_hex_with_padding($response);
						
						//	Set parameters for reordering and build reordering condition
						$V2 = intval(coap_hex_to_string($response_obj->get_option_value(6)));
						$T2 = time();
						$reordering_condition = (($V1 < $V2 && $V2 - $V1 < pow(2, 32)) || ($V1 > $V2 && $V1 - $V2 > pow(2, 32)) || ($T2 > $T1 + 128));
						
						//	Only take care of the message if the message contains the correct token or the reordering condition is met (cfr. CoAP observe draft)
						if($response_obj->get_token() == $request_token && $reordering_condition){
							//	notify using module that there are no difficulties at the moment
							$function_name = $this->module . '_receive_error';
							$function_name('none', $this->ip, $this->resource);
							
							//	Call hook_receive_notification of the using module
							$function_name = $this->module . '_receive_notification';
							$function_name($response_obj);
							
							//	Send ACK back to server if the response message was of type CON
							if($response_obj->is_confirmable()){
								$ack = coap_library_generate_acknowledgement(coap_library_string_to_hex_with_padding($response));
								$ack = pack('H*', $ack);
								$returnvalue = fwrite($socket, $ack);
							}
						}
						else if($response_obj->get_token() != $request_token && $response_obj->is_confirmable()){
							//	Send a reset to inform the server that the client has no interest in notifications with this token
							$reset = coap_library_generate_reset(coap_library_string_to_hex_with_padding($response));
							$reset = pack('H*', $reset);
							$returnvalue = fwrite($socket, $reset); // Write to socket
						}
						
						//	Wait for next notification
						$response = 'empty';
						$response = fread($socket, 128); // Read from socket
						
						//	Keep resending the packet and multiplying the timeout if something went wrong (exponential backoff)
						while($this->retransmit_counter < $this->MAX_RETRANSMIT && ($response == '' || $response == 'empty')){
							//	Notify using module that something went wrong
							$function_name = $this->module . '_receive_error';
							$function_name('delay', $this->ip, $this->resource);
							
							//	Rebuild and send the packet
							$coap_factory = new CoAPMessageFactory($this->module, $this->ip, $this->resource, $this->separate_response);
							$this->message_hex = $coap_factory->create_observe_get_request($this->resource);
							$message = pack('H*', $this->message_hex);
							$returnvalue = fwrite($socket, $message); // wegschrijven naar udp-socket
							if($returnvalue != FALSE){
								$response = fread($socket, 128); // lezen van udp-socket
							}
							
							//	Set new timeout
							$this->retransmit_counter++;
							$timeout *= 2;
							stream_set_timeout($socket, 0, $timeout*1000);
						}
						
						//	Set variables for reordering
						$V1 = $V2;
						$T1 = $T2;
						$V2 = intval(coap_hex_to_string($response_obj->get_option_value(6)));
						$T2 = time();
						
						$response_obj = new CoAPMessage($this->module, $this->ip, $this->resource, coap_library_string_to_hex_with_padding($response));
						$must_observe = $this->observe_is_necessary(); //	Check if resource should still be observed
					}
					
					//	Check if the client has aborted the observe or something else went wrong
					if($must_observe > 0){
						//	Notify the using module that something went wrong
						$function_name = $this->module . '_receive_error';
						$function_name('broken', $this->ip, $this->resource);
					}
					else{
						//	Send a reset to inform the server that the client has lost interest
						$reset = coap_library_generate_reset(coap_library_string_to_hex_with_padding($response));
						$reset = pack('H*', $reset);
						$returnvalue = fwrite($socket, $reset); // wegschrijven naar udp-socket
					}
				}
				else if($response_obj->separate_response && ($response_obj->get_payload() == null || $response_obj->get_payload() == '') && $request_message_id == $response_obj->get_message_id()){
					//	Resource uses separate response and has returned an empty ACK
					$response = 'empty';
					stream_set_timeout($socket, 300, 0);
					$response = fread($socket, 512); // Read from socket
					if($response != 'empty' && $response != ''){
						$response_obj = new CoAPMessage($this->module, $this->ip, $this->resource, coap_library_string_to_hex_with_padding($response));
						if($response_obj->is_confirmable()){
							$ack = coap_library_generate_acknowledgement(coap_library_string_to_hex_with_padding($response));
							$ack = pack('H*', $ack);
							$returnvalue = fwrite($socket, $ack);
						}
						fclose($socket); // Close the socket
						return $response_obj;
					}
					else{
						$function_name = $this->module . '_receive_error';
						$function_name('broken', $this->ip, $this->resource);
						fclose($socket); // Close the socket
						return null;
					}
				}
				else{
					//	Resource should not be observed
					fclose($socket); // Close the socket
					return $response_obj;
				}
			}
			//	Only notify the using module that there went something wrong if the message type of the request was CON
			else if($this->is_confirmable()){
				$function_name = $this->module . '_receive_error';
				$function_name('broken', $this->ip, $this->resource);
			}
			fclose($socket); // Close the socket
		}
		else{
			// Socket could not be opened
			$function_name = $this->module . '_receive_error';
			$function_name('unreachable', $this->ip, $this->resource);
		}
		
		$this->stop_observing();
		
		return null;
	}
	
	// checks if there are still users observing the resource
	private function observe_is_necessary(){
		$query = db_select('coap_library_observe_resources', 'resources');
		$query
			->fields('resources', array('module', 'uri'))
			->condition('uri', $this->ip . '/' . $this->resource, '=')
			->condition('module', $this->module, '=');
		$result = $query->execute();
		return $result->rowCount();
	}
	
	// 	hook_stop_observing, must be implemented by modules using this library
	// 	Called when the observe stops, for whatever reason
	//	Calls hook_stop_observers to give the user of this module the chance to handle this event
	public function stop_observing(){
		$num_deleted = db_delete('coap_library_observe_resources')
			->condition('uri', $this->ip . '/' . $this->resource, '=')
			->condition('module', $this->module, '=')
			->execute();
			
		background_process_start($this->module . '_stop_observers', $this->ip, $this->resource);
	}
	
	// Retrieves the content-format from the message
	// Returns null if content-format option is not present
	public function get_content_format(){
		$format_nr = 0;
		$format_nr += intval($this->get_option_value(12));
		if($format_nr < 0){
			$format_nr = 0;
		}
		return $this->format_mapping[$format_nr];
	}
	
	// Retrieves the textual representation of the code of the message
	public function get_message_code(){
		$code_nr = hexdec(substr($this->message_hex, 2, 2));
		return $this->response_code_mapping[$code_nr];
	}
	
	// Checks if this message is confirmable or not
	// Returns boolean
	public function is_confirmable(){
		return (substr($this->message_hex, 0, 1) == '4');
	}
	
	//	Returns the token from this message
	//	Returns null if the token length == 0
	public function get_token(){
		$token_length = intval(coap_library_hex_to_string(substr($this->message_hex, 1, 1)));
		if($token_length <= 0){
			return null;
		}
		return substr($this->message_hex, 8, $token_length*2);
	}
	
	//	Returns the message id from this message
	public function get_message_id(){
		return substr($this->message_hex, 4, 4);
	}
	
}


/////////////////////////////////
//	CoAP Message Factory Class //
/////////////////////////////////

class CoAPMessageFactory{

	private $module = null;
	private $ip;
	private $resource;
	private $separate_response;
	
	// Constructor with given module name
	public function __construct($module, $ip, $resource, $separate_response){
        $this->module = $module;  
		$this->ip = $ip;
		$this->resource = $resource;
		$this->separate_response = $separate_response;
    }

	// Generates a valid message ID, message id's get incremented with each function call
    private function generate_message_id(){
		$message_id = variable_get('current_message_id', null);
		if(!isset($message_id)){
			$message_id = rand(0, 65535);
		}
		else{
			$message_id++;
			$message_id % 65536;
		}
		variable_set('current_message_id', $message_id);
		$message_id = dechex($message_id);
		$message_id_string = strval($message_id);
		$message_id_string = str_pad($message_id_string, 4, "0", STR_PAD_LEFT);
		return $message_id_string;
	}
	
	// Generates a valid token with given length, tokens get incremented with each function call
	private function generate_token($token_length){
		if(!is_int($token_length)){
			return null;
		}
		$token = variable_get('current_token', null);
		if(!isset($token)){
			$token = rand(0, 65535);
		}
		else{
			$token++;
			$token % 65536;
		}
		variable_set('current_token', $token);
		$token = dechex($token);
		$token_string = strval($token);
		$token_string = str_pad($token_string, $token_length*2, "0", STR_PAD_LEFT);
		return substr($token_string, 0, $token_length*2);
	}
	
	// Generates an empty CoAP message
	public function create_empty_coap_message(){
		return new CoAPMessage($this->module, $this->ip, $this->resource, $this->separate_response);
	}
	
	// Generates a basic CoAP message consisting of only the header and optional token, $type = {CON, NON, ACK, RST}, $method = {GET, PUT, POST, DELETE}
	public function create_message_with_header_and_token($type, $method, $token_length){
		if(!is_int($token_length) || $token_length < 0 || $token_length > 8){
			return null;
		}
		$hex = null;
		switch ($type) {
			case "CON":
				$hex .= "4";
				break;
			case "NON":
				$hex .= "5";
				break;
			case "ACK":
				$hex .= "6";
				break;
			case "RST":
				$hex .= "7";
				break;
			default:
				return null;
		}
		$hex .= $token_length;
		switch ($method) {
			case "EMPTY":
				$hex .= "00";
				break;
			case "GET":
				$hex .= "01";
				break;
			case "POST":
				$hex .= "02";
				break;
			case "PUT":
				$hex .= "03";
				break;
			case "DELETE":
				$hex .= "04";
				break;
			default:
				return null;
		}
		$hex .= $this->generate_message_id();
		$hex .= $this->generate_token($token_length);
		return new CoAPMessage($this->module, $this->ip, $this->resource, $this->separate_response, $hex);
	}
	
	// Generates a basic CoAP GET request with optional given URI path
	public function create_basic_get_request($uri_path){
		$coap_message = $this->create_message_with_header_and_token("CON", "GET", 0);
		if(isset($uri_path) && $uri_path != ''){
		variable_set('get_response', 'uri path');
			$coap_message->add_option(11, $uri_path);
		}
		return $coap_message;
	}
	
	// Generates a CoAP observe GET request with given URI-path
	public function create_observe_get_request($uri_path){
		$coap_message = $this->create_message_with_header_and_token("CON", "GET", 2);
		$coap_message->add_option(6, '');
		$coap_message->add_option(11, 'obs');
		return $coap_message;
	}
	
	// Creates a message with the given hexadecimal message
	public function create_message_with_given_message($message){
		return new CoAPMessage($this->module, $this->ip, $this->resource, $this->separate_response, $message);
	}
	
	// Creates a discovery request which depends on response
	public function create_discovery_request($response){
		$coap_message = $this->create_message_with_header_and_token("CON", "GET", 0);
		$coap_message->add_option(11,'.well-known');
		$coap_message->add_option(11,'core');
		
		if($response){
			$response_option_value = $response->get_option_value(23);
			$option_value_length = strlen($response_option_value);
			//2 => xx
			//4 => xx xx
			//6 => xx xx xx
			//...
			
			$first_hex_digits = substr($response_option_value, 0, $option_value_length-1);
			$carry = coap_library_increment_hex($first_hex_digits);
			
			$last_hex_digit = substr($response_option_value, $option_value_length-1, 1);
			$size = 0;			
			if(coap_library_get_bit_from_hex($last_hex_digit,2) == 1) $size += 4;
			if(coap_library_get_bit_from_hex($last_hex_digit,1) == 1) $size += 2;
			if(coap_library_get_bit_from_hex($last_hex_digit,0) == 1) $size += 1;
			$last_hex_digit = dechex($size);
			
			//build option 23 value for request
			$request_option_value = '';
			if($carry) $request_option_value .= '01';
			$request_option_value .= $first_hex_digits . $last_hex_digit;
			$coap_message->add_option(23,coap_library_hex_to_string($request_option_value));
		}
		return $coap_message;
	}

}  