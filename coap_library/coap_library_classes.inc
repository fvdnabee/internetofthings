<?php


////////////////////////////////
// CoAP Message Factory Class //
////////////////////////////////

class CoAPMessage {
	
    private $message_hex = "";
	private $module = null;
	private $ip;
	private $resource;
	
	// Constructor with optional given message, expects hexadecimal message
	public function __construct($module, $ip, $resource, $message = ''){
        $this->message_hex = $message;
		$this->module = $module;
		$this->ip = $ip;
		$this->resource = $resource;
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
	
	// Adds an option with given number and value to the given message, returns null if message is smaller than 4 bytes
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
		if($option_number > 268){
			$mid .= "e";
		}
		else if($option_number > 13){
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
		if($option_number > 268){
			$temp = coap_library_string_to_hex_with_padding($option_number-269-$current_number);
			str_pad($temp, 4, "0", STR_PAD_LEFT);
			$mid .= $temp;
		}
		else if($option_number > 12){
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
		if(!strpos($this->message_hex, 'ff', 0)){
			return null;
		}
		return $message . 'ff' . coap_library_string_to_hex_with_padding($payload);
	}
	
	// Gets the value of the option with given number, if present. Returns -1 if not present.
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
	
	// Retrieves the max-age value, if present, from a message. Returns null if not present
	public function get_max_age(){
		$max_age_hex = $this->get_option_value(14);
		return hexdec($max_age_hex);
	}
	
	// Retrieves the payload, if present, from a message. Returns null if not present
	public function get_payload(){
		if(strlen($this->message_hex) < 8){
			return null;
		}
		$pos = strpos($this->message_hex, 'ff', 0);
		if(!$pos){
			return null;
		}
		return coap_library_hex_to_string(substr($this->message_hex, $pos+2));
	}
	
	// Returns response in binary form or null if there was no response or sending failed
	// Using this function requires implementation of the hook module_name_receive_error().
	public function send_message(){
		$observe = ($this->get_option_value(6) == -1) ? 0 : 1;
		if($observe){
			$query = db_select('coap_library_observe_resources', 'resources');
			$query
				->fields('resources', array('module', 'uri'))
				->condition('uri', $this->ip . '/' . $this->resource, '=')
				->condition('module', $this->module, '=');
			$result = $query->execute();
			if($result->rowCount() != 0){
				return null;
			}
			
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
		$socket = pfsockopen("udp://[" . $this->ip . "]", $port, $errno, $errstr); // udp-socket openen
		if ($socket) {
			$function_name = $this->module . '_receive_error';
			$function_name('none', $this->ip, $this->resource);
			stream_set_timeout($socket, 5, 0);
			$retries = 0;
			$message = pack('H*', $this->message_hex);
			$returnvalue = fwrite($socket, $message); // wegschrijven naar udp-socket
			if($returnvalue != FALSE){
				$response = fread($socket, 128); // lezen van udp-socket
			}
			while($retries < 5 && ($response == '' || $response == 'empty')){
				$function_name = $this->module . '_receive_error';
				$function_name('delay', $this->ip, $this->resource);
				$returnvalue = fwrite($socket, $message); // wegschrijven naar udp-socket
				if($returnvalue != FALSE){
					$response = fread($socket, 128); // lezen van udp-socket
				}
				$retries++;
			}
			if($response != '' && $response != 'empty'){
				$response_obj = new CoAPMessage($this->module, $this->ip, $this->resource, coap_library_string_to_hex_with_padding($response));
				$function_name = $this->module . '_receive_error';
				$function_name('none', $this->ip, $this->resource);
				if($observe){
					// Observe
					$must_observe = $this->observe_is_necessary();
					
					stream_set_timeout($socket, intval($response_obj->get_max_age()), 0);
					while($must_observe > 0 && $response != '' && $response != 'empty'){
						$response_hex = coap_library_string_to_hex_with_padding($response);
						$function_name = $this->module . '_receive_error';
						$function_name('none', $this->ip, $this->resource);
						$function_name = $this->module . '_receive_response';
						$function_name($response_obj);
						$ack = coap_library_generate_acknowledgement(coap_library_string_to_hex_with_padding($response));
						$ack = pack('H*', $ack);
						$returnvalue = fwrite($socket, $ack);
						$retries = 0;
						$response = 'empty';
						$response = fread($socket, 128); // lezen van udp-socket
						while($retries < 3 && ($response == '' || $response == 'empty')){
							$function_name = $this->module . '_receive_error';
							$function_name('delay', $this->ip, $this->resource);
							$coap_factory = new CoAPMessageFactory($this->module, $this->ip, $this->resource);
							$this->message_hex = $coap_factory->create_observe_get_request($this->resource);
							$message = pack('H*', $this->message_hex);
							$returnvalue = fwrite($socket, $message); // wegschrijven naar udp-socket
							if($returnvalue != FALSE){
								$response = fread($socket, 128); // lezen van udp-socket
							}
							$retries++;
						}
						$response_obj = new CoAPMessage($this->module, $this->ip, $this->resource, coap_library_string_to_hex_with_padding($response));
						$must_observe = $this->observe_is_necessary();
					}
					if($must_observe > 0){
						$function_name = $this->module . '_receive_error';
						$function_name('broken', $this->ip, $this->resource);
					}
					else{
						$reset = coap_library_generate_reset(coap_library_string_to_hex_with_padding($response));
						$reset = pack('H*', $reset);
						$returnvalue = fwrite($socket, $reset); // wegschrijven naar udp-socket
					}
				}
				else{
					return $response_obj;
				}
			}
			else{
				$function_name = $this->module . '_receive_error';
				$function_name('broken', $this->ip, $this->resource);
			}
			fclose($socket);
		}
		else{
			$function_name = $this->module . '_receive_error';
			$function_name('unreachable', $this->ip, $this->resource);
		}
		
		$this->stop_observing();
		
		return null;
	}
	
	private function observe_is_necessary(){
		$query = db_select('coap_library_observe_resources', 'resources');
		$query
			->fields('resources', array('module', 'uri'))
			->condition('uri', $this->ip . '/' . $this->resource, '=')
			->condition('module', $this->module, '=');
		$result = $query->execute();
		return $result->rowCount();
	}
	
	public function stop_observing(){
		$num_deleted = db_delete('coap_library_observe_resources')
			->condition('uri', $this->ip . '/' . $this->resource, '=')
			->condition('module', $this->module, '=')
			->execute();
			
		background_process_start($this->module . '_stop_observers', $this->ip, $this->resource);
	}
	
}


/////////////////////////////////
//	CoAP Message Factory Class //
/////////////////////////////////

class CoAPMessageFactory{

	private $module = null;
	private $ip;
	private $resource;
	
	// Constructor with given module name
	public function __construct($module, $ip, $resource){
        $this->module = $module;  
		$this->ip = $ip;
		$this->resource = $resource;
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
		return new CoAPMessage($this->module, $this->ip, $this->resource);
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
		return new CoAPMessage($this->module, $this->ip, $this->resource, $hex);
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
		return new CoAPMessage($this->module, $this->ip, $this->resource, $message);
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
			//variable_set("response_string_0", $size);			
			$last_hex_digit = dechex($size);
			
			//build option 23 value for request
			$request_option_value = '';
			if($carry) $request_option_value .= '01';
			$request_option_value .= $first_hex_digits . $last_hex_digit;
			variable_set("response_string_0", $request_option_value);
			$coap_message->add_option(23,$request_option_value);
		}
		return $coap_message;
	}
}  