<?php

//	DESIGN NOTE: 	Different options with the same option number should be added together, divided by a slash (/)
//					Example: instead of add_option(11, 'seg1') and add_option(11, 'seg2'), use add_option(11, 'seg1/seg2')


////////////////////////////////
// CoAP Message Factory Class //
////////////////////////////////

class CoAPMessage {
	
    //	CONFIG
	private $module = null;
	private $ip;
	private $resource;
	
	//	HEADER
	private $version = 1;
	private $type;
	private $code;
	private $message_id; //	IMPORTANT: message ID is kept in numeric form, not hexadecimal!
	
	//	CONTENT
	private $options = array();
	private $payload = '';
	private $token = '';
	
	//	CONSTANTS
	private $ACK_TIMEOUT = 2000; // 2 seconds
	private $MAX_RETRANSMIT = 4;
	private $RANDOM_FACTOR = 1.5;
	private $NSTART = 1;
	private $DEFAULT_LEISURE = 5000; // 5 seconds
	private $PROBING_RATE = 5; // 1 Byte/second
	
	//	MAPPING
	private $response_code_mapping = array(
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
	
	private $format_mapping = array(
		0 => 'text/plain',
		40 => 'application/link-format',
		41 => 'application/xml',
		42 => 'application/octet-stream',
		47 => 'application/exi',
		50 => 'application/json'
	);
	
	//	EXPONENTIAL BACKOFF
	private $retransmit_counter = 0;
	
	// Constructor
	public function __construct($module, $ip, $resource, $type, $code, $token_length, $message_id = null, $token = null){
		
		//STORE ATTRIBUTES
		$this->module = $module;
		$this->ip = $ip;
		$this->resource = $resource;
		$this->type = $type;
		$this->code = $code;
		
		//	GENERATE TOKEN AND MESSAGE ID
		if($message_id == null){
			$this->message_id = coap_library_generate_message_id();
			$this->token = coap_library_generate_token($token_length);
		}
		else{
			$this->message_id = $message_id;
			if($token == null){
				$this->token = coap_library_generate_token($token_length);
			}
			else{
				$this->token = $token;
			}
		}
		
    }
	
	// Getter for the hexadecimal message
	public function get_hex_message(){
		$hex = null;
		
		//	Add version + type
		$type_nr;
		switch ($this->type) {
			case "CON":
				$type_nr = 0;
				break;
			case "NON":
				$type_nr = 1;
				break;
			case "ACK":
				$type_nr = 2;
				break;
			case "RST":
				$type_nr = 3;
				break;
			default:
				return null;
		}
		$hex .= dechex($this->version*4 + $type_nr);
		
		//	Add token length
		$hex .= dechex(strlen($this->token)/2);
		
		//	Add message code
		$hex .= str_pad(dechex($this->code), 2, "0", STR_PAD_LEFT);
		
		//	Add message id
		$message_id_string = strval(dechex($this->message_id));
		$message_id_string = str_pad($message_id_string, 4, "0", STR_PAD_LEFT);
		$hex .= $message_id_string;
		
		//	Add token
		$hex .= $this->token;
		
		//	Add options
		ksort($this->options);	//	Order options array by growing option deltas to keep option deltas in the hex message consistent
		foreach($this->options as $option_nr => $option_value){
			$values = explode('/', $option_value);
			foreach($values as $value){
				coap_library_add_option_to_hex_message($hex, $option_nr, $value);
			}
		}
		
		//	Add payload
		coap_library_add_payload_to_hex_message($hex, $this->payload);
		
		return $hex;
	}
		
	// Getter for the ip of the device
	public function get_ip(){
		return $this->ip;
	}
	
	// Getter for the uri path of the resource
	public function get_resource(){
		return $this->resource;
	}
	
	//	Adds an option to the message object with given number and value
	public function add_option($nr, $value){
		$this->options[$nr] = $value;
	}
	
	//	Getter for option with given number
	//	Retrieves option value
	//	Return null if option is not present in this message
	public function get_option_value($nr){
		if(isset($this->options[$nr])){
			return $this->options[$nr];
		}
		else{
			return null;
		}
	}
	
	//	Setter for the payload attribute
	//	The given payload is expected to be hexadecimal
	public function set_payload($payload){
		$this->payload = $payload;
	}
	
	// Retrieves the max-age value, if present, from this message
	// Returns 0 if no max age option is included
	public function get_max_age(){
		$max_age_hex = $this->get_option_value(14);
		if(!isset($max_age_hex) || $max_age_hex == null || !$max_age_hex){
			return 0;
		}
		return hexdec($max_age_hex);
	}
		
	//	Retrieves the payload, if present, from this message
	//	Returns null if no payload is present
	//	Returns payload in hexadecimal form
	public function get_payload(){
		return $this->payload;
	}
	
	// 	Returns a response object with the response if the request doesn't have an observe option
	//	Returns null if the request holds an observe option
	//	Returns null if something went wrong
	//	Calls hook_receive_notification for notifications that have arrived from an observable resource
	//	Calls hook_receive_notification if something went wrong
	public function send_message(){
		//	Create CoAP factory
		$coap_factory = new CoAPMessageFactory($this->module, $this->ip, $this->resource);
	
		//	Determine if request has an observe option
		$observe = ($this->get_option_value(6) === null) ? 0 : 1;
		
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
		
		//	Give the response an initial value
		$response = 'empty';
		
		// Open UDP-socket
		$port = 5683;
		$socket = pfsockopen("udp://[" . $this->ip . "]", $port, $errno, $errstr);
		if($socket){
			//	Socket opened
			//	notify using module that there are no difficulties at the moment
			$function_name = $this->module . '_receive_error';
			$function_name('none', $this->ip, $this->resource);
			
			//	Set initial timeout
			$timeout = rand($this->ACK_TIMEOUT, $this->ACK_TIMEOUT*$this->RANDOM_FACTOR); 
			stream_set_timeout($socket, 0, $timeout*1000); // stream_set_timeout expects third parameter to be in MICROseconds
			$this->retransmit_counter = 0;
			
			//	Send CoAP packet
			$binary_message = pack('H*', $this->get_hex_message());
			$returnvalue = fwrite($socket, $binary_message); //	Write to socket
			
			//	Stop this function if the message is an acknowledgement or a reset message
			if($this->type == "ACK" || $this->type == "RST"){
				return null;
			}
			
			//	Check for response if the message is confirmable
			if($this->is_confirmable() || $observe){
				//	Message type = CON or sent message had an observe option
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
					$binary_message = pack('H*', $this->get_hex_message());
					$returnvalue = fwrite($socket, $binary_message); //	Write to socket
					if($returnvalue != FALSE){
						$response = fread($socket, 512); //	Read from socket
					}
				}
			}
			
			//	Check if response isn't empty
			if($response != '' && $response != 'empty'){
				$response_obj = $coap_factory->create_message_with_given_hex_message(coap_library_string_to_hex_with_padding($response)); //	Put response in CoAPMessage-object
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
					
					//	Set timeout to 24 hours, server should at least send a notification every 24 hours
					stream_set_timeout($socket, 0, 24*60*60*1000);
					
					//	Set initial values for reordering
					$V1 = 0;
					$T1 = time();
					
					//	Keep receiving notifications while observe is still necessary
					while($must_observe > 0 && $response != '' && $response != 'empty'){
						//	Resource should still be observed
						
						//	Set parameters for reordering and build reordering condition
						$V2 = hexdec($response_obj->get_option_value(6));
						$T2 = time();
						$reordering_condition = (($V1 < $V2 && $V2 - $V1 < pow(2, 32)) || ($V1 > $V2 && $V1 - $V2 > pow(2, 32)) || ($T2 > $T1 + 128));
						
						//	Only take care of the message if the message contains the correct token or the reordering condition is met (cfr. CoAP observe draft)
						if($response_obj->get_token() == $this->token && $reordering_condition){
							//	notify using module that there are no difficulties at the moment
							$function_name = $this->module . '_receive_error';
							$function_name('none', $this->ip, $this->resource);
							
							//	Call hook_receive_notification of the using module
							$function_name = $this->module . '_receive_notification';
							$function_name($response_obj);
							
							//	Send ACK back to server if the response message was of type CON
							if($response_obj->is_confirmable()){
								$ack = $response_obj->generate_acknowledgement();
								$ack_message = pack('H*', $ack->get_hex_message());
								$returnvalue = fwrite($socket, $ack_message); // wegschrijven naar udp-socket
							}
						}
						else if($response_obj->get_token() != $this->token && $response_obj->is_confirmable()){
							//	Send a reset to inform the server that the client has no interest in notifications with this token
							$reset = $response_obj->generate_reset();
							$reset_message = pack('H*', $reset->get_hex_message());
							$returnvalue = fwrite($socket, $reset_message); // wegschrijven naar udp-socket
						}
						
						//	Wait for next notification
						$response = 'empty';
						$response = fread($socket, 512); // Read from socket
						
						//	Set variables for reordering
						$V1 = $V2;
						$T1 = $T2;
						$V2 = hexdec($response_obj->get_option_value(6));
						$T2 = time();
						
						$response_obj = $coap_factory->create_message_with_given_hex_message(coap_library_string_to_hex_with_padding($response)); //	Put response in CoAPMessage-object
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
						$reset = $response_obj->generate_reset();
						$reset_message = pack('H*', $reset->get_hex_message());
						$returnvalue = fwrite($socket, $reset_message); // wegschrijven naar udp-socket
					}
				}
				else if(($response_obj->get_payload() == null || $response_obj->get_payload() == '') && $this->message_id == $response_obj->get_message_id() && $this->code == 0){
					//	Resource has returned an empty ACK
					$response = 'empty';
					stream_set_timeout($socket, 20, 0); //	Timeout for separate response currently set to 20 seconds, but can be chosen
					$response = fread($socket, 512); // Read from socket
					if($response != 'empty' && $response != ''){
						$response_obj = $coap_factory->create_message_with_given_hex_message(coap_library_string_to_hex_with_padding($response)); //	Put response in CoAPMessage-object
						
						//	Check if the message is confirmable
						if($response_obj->is_confirmable()){
							//	Message is confirmable
							//	Send an acknowledgement back
							$ack = $response_obj->generate_acknowledgement();
							$ack_message = pack('H*', $ack->get_hex_message());
							$returnvalue = fwrite($socket, $ack_message); // wegschrijven naar udp-socket
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
					
					//	Check if the response consists of blocks and get the following blocks if necessary
					$response_block_value = $response_obj->get_option_value(23);	//	REKENING HOUDEN MET OPTIE 28 ------------------------------------------------------------------------------
					$request_block_value = $this->get_option_value(23);
					if($response_block_value != null && $request_block_value == null){
						//	Response comes in blocks
						$this->get_next_responses_in_blocks($response_obj);
					}
					
					return $response_obj;
				}
			}
			
			//	Only notify the using module that there went something wrong if the message type of the request was CON
			else if($this->is_confirmable()){
				$function_name = $this->module . '_receive_error';
				$function_name('broken', $this->ip, $this->resource);
			}
			
			//	Close the socket
			fclose($socket);
		}
		
		//	The socket could not be opened
		else{
			// Socket could not be opened
			$function_name = $this->module . '_receive_error';
			$function_name('unreachable', $this->ip, $this->resource);
		}
		
		//	Make sure observing stops when this function ends
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
	
	//	Retrieves the textual representation of the code of this message
	public function get_message_code(){
		return $this->response_code_mapping[$this->code];
	}
	
	//	Retrieves the numeric code of this message
	public function get_numeric_message_code(){
		return $this->code;
	}
	
	//	Checks if this message is confirmable or not
	//	Returns boolean
	public function is_confirmable(){
		return ($this->type == "CON");
	}
	
	//	Returns the token from this message
	//	Can be an empty string
	public function get_token(){
		return $this->token;
	}
	
	//	Returns the message id from this message
	public function get_message_id(){
		return $this->message_id;
	}
	
	//	Returns the payload in textual form
	public function get_payload_text(){
		return coap_library_hex_to_string($this->payload);
	}
	
	//	Gets the next blocks following to the given response and appends the resulting payloads
	private function get_next_responses_in_blocks(&$response_obj){
		$current_response = $response_obj;
		
		//	Determine if there are still payloads to fetch
		$block_option_value = $current_response->get_option_value(23);
		switch (strlen($block_option_value)) {
			case 2:
				$payloads_left = coap_library_get_bit_from_hex(substr($block_option_value, 1, 1), 3);
				break;
			case 4:
				$payloads_left = coap_library_get_bit_from_hex(substr($block_option_value, 3, 1), 3);
				break;
			case 6:
				$payloads_left = coap_library_get_bit_from_hex(substr($block_option_value, 5, 1), 3);
				break;
			default:
				return null;
		}
		
		//	Get payloads and append them
		while($payloads_left){
			//	Build and send the next request
			$response_option_value = $current_response->get_option_value(23);
			$option_value_length = strlen($response_option_value);
			
			$first_hex_digits = substr($response_option_value, 0, $option_value_length-1);
			$carry = coap_library_increment_hex($first_hex_digits);
			
			$last_hex_digit = substr($response_option_value, $option_value_length-1, 1);
			$size = 0;			
			if(coap_library_get_bit_from_hex($last_hex_digit,2) == 1) $size += 4;
			if(coap_library_get_bit_from_hex($last_hex_digit,1) == 1) $size += 2;
			if(coap_library_get_bit_from_hex($last_hex_digit,0) == 1) $size += 1;
			$last_hex_digit = dechex($size);
			
			//	Build option 23 value for request
			$request_option_value = '';
			if($carry) $request_option_value .= '01';
			$request_option_value .= $first_hex_digits . $last_hex_digit;
			$this->add_option(23, $request_option_value);
			
			//	Send the request
			$this->message_id = coap_library_generate_message_id();
			$current_response = $this->send_message();
			
			//	Append payload and check if there is a next block
			if($current_response){
				$option_value = $current_response->get_option_value(23);
				if($option_value != null){
					$payloads_left = coap_library_get_bit_from_hex(substr($option_value,strlen($option_value)-1,1),3);
					$response_obj->set_payload($response_obj->get_payload() . $current_response->get_payload());
					
					//	Determine if there are still payloads to fetch
					switch (strlen($option_value)) {
						case 2:
							$payloads_left = coap_library_get_bit_from_hex(substr($option_value, 1, 1), 3);
							break;
						case 4:
							$payloads_left = coap_library_get_bit_from_hex(substr($option_value, 3, 1), 3);
							break;
						case 6:
							$payloads_left = coap_library_get_bit_from_hex(substr($option_value, 5, 1), 3);
							break;
						default:
							return null;
					}
				}
				else{
					$payloads_left = false;
				}
			}
			else{
				$payloads_left = false;
			}
		}
	}
	
	//	Add an URI path option to this message
	//	Expects a normal ASCII string as option value
	//  Multiple uri_paths can be combined by placing double slashes (//) between them.
	//  This allows the use of a single slash in the uri_path.
	public function add_uri_path($uri_path){
		$paths = explode("//", $uri_path);
		$hex_paths = array();
		foreach($paths as $path){
			array_push($hex_paths, coap_library_string_to_hex_with_padding($path));
		}
		$option_value_hex = implode("/", $hex_paths);
		$this->add_option(11, $option_value_hex);
	}
	
	//	Generates an acknowledgement to this message
	//	Returns a CoAPMessage-object
	public function generate_acknowledgement(){
		$factory = new CoAPMessageFactory($this->module, $this->ip, $this->resource);
		$ack = $factory->create_message_with_header_and_token("ACK", "EMPTY", 0, $this->message_id);
		return $ack;
	}
	
	//	Generates a reset to this message
	//	Returns a CoAPMessage-object
	public function generate_reset(){
		$factory = new CoAPMessageFactory($this->module, $this->ip, $this->resource);
		$reset = $factory->create_message_with_header_and_token("RST", "EMPTY", 0, $this->message_id);
		return $reset;
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
	
	// 	Generates a basic CoAP message consisting of only the header and optional token, $type = {CON, NON, ACK, RST}, $method = {GET, PUT, POST, DELETE}
	//	Returns null if the token length is invalid
	public function create_message_with_header_and_token($type, $method, $token_length, $message_id = null, $token = null){
		//	Check if token length is valid
		if(!is_int($token_length) || $token_length < 0 || $token_length > 8){
			return null;
		}
		
		//	Determine the numeric code
		$code;
		switch ($method) {
			case "EMPTY":
				$code = 0;
				break;
			case "GET":
				$code = 1;
				break;
			case "POST":
				$code = 2;
				break;
			case "PUT":
				$code = 3;
				break;
			case "DELETE":
				$code = 4;
				break;
			default:
				return null;
		}
		
		//	Create a message object and return it
		return new CoAPMessage($this->module, $this->ip, $this->resource, $type, $code, $token_length, $message_id, $token);
	}
	
	// Generates a basic CoAP GET request with optional given URI path
	public function create_basic_get_request($uri_path){
		$coap_message = $this->create_message_with_header_and_token("CON", "GET", 0);
		if(isset($uri_path) && $uri_path != ''){
			$coap_message->add_uri_path($uri_path);
		}
		return $coap_message;
	}
	
	public function create_basic_post_request($uri_path){
		$coap_message = $this->create_message_with_header_and_token("CON", "POST", 0);
		if(isset($uri_path) && $uri_path != ''){
			$coap_message->add_uri_path($uri_path);
		}
		return $coap_message;
	}
	
	public function create_basic_put_request($uri_path){
		$coap_message = $this->create_message_with_header_and_token("CON", "PUT", 0);
		if(isset($uri_path) && $uri_path != ''){
			$coap_message->add_uri_path($uri_path);
		}
		return $coap_message;
	}
	
	public function create_basic_delete_request($uri_path){
		$coap_message = $this->create_message_with_header_and_token("CON", "DELETE", 0);
		if(isset($uri_path) && $uri_path != ''){
			$coap_message->add_uri_path($uri_path);
		}
		return $coap_message;
	}	
	
	// Generates a CoAP observe GET request with given URI-path
	public function create_observe_get_request($uri_path){
		$coap_message = $this->create_message_with_header_and_token("CON", "GET", 2);
		if(isset($uri_path) && $uri_path != ''){
			$coap_message->add_uri_path($uri_path);
		}
		$coap_message->add_option(6, '');
		return $coap_message;
	}
	
	// Creates a message with the given hexadecimal message
	public function create_message_with_given_hex_message($hex_message){
		//	Determine type (textual representation)
		$type = coap_library_get_numeric_type_from_hex_message($hex_message);
		switch ($type) {
			case 0:
				$type = "CON";
				break;
			case 1:
				$type = "NON";
				break;
			case 2:
				$type = "ACK";
				break;
			case 3:
				$type = "RST";
				break;
			default:
				return null;
		}
		
		$code = coap_library_get_numeric_message_code_from_hex_message($hex_message);
		$token = coap_library_get_token_from_hex_message($hex_message);
		$token_length = strlen($token)/2;
		$message_id = hexdec(coap_library_get_message_id_from_message($hex_message));
		$message = new CoAPMessage($this->module, $this->ip, $this->resource, $type, $code, $token_length, $message_id, $token);
		
		//	Add options
		if(strlen($hex_message) >= 8){
			//	Message has a valid header
			if(strlen($hex_message) >= 8+($token_length*2)){
				//	Message has a valid token, if present at all
				$pos = 8 + ($token_length*2);
				$current_number = 0;
				if($pos < strlen($hex_message)){
					$extra = 0;
					$option_delta = hexdec(substr($hex_message, $pos, 1));
					if($option_delta == 13){
						$option_delta = hexdec(substr($hex_message, $pos+2, 2))-13;
						$extra += 2;
					}
					else if($option_delta == 14){
						$option_delta = hexdec(substr($hex_message, $pos+2, 4))-269;
						$extra += 4;
					}
					$option_length = hexdec(substr($hex_message, $pos+1, 1));
					if($option_length == 13){
						$option_length = hexdec(substr($hex_message, $pos+2+$extra, 2))-13;
						$extra += 2;
					}
					else if($option_length == 14){
						$option_length = hexdec(substr($hex_message, $pos+2+$extra, 4))-269;
						$extra += 4;
					}
					$option_length *= 2;
				}
				while($pos < strlen($hex_message) && substr($hex_message, $pos, 2) != "ff"){
					$current_number += $option_delta;
					
					//	Add option number and option value to message object
					$message->add_option($current_number, substr($hex_message, $pos+2, $option_length));
					
					$pos += 2 + $extra + $option_length;
					if($pos < strlen($hex_message)){
						$extra = 0;
						$option_delta = hexdec(substr($hex_message, $pos, 1));
						if($option_delta == 13){
							$option_delta = hexdec(substr($hex_message, $pos+2, 2))-13;
							$extra += 2;
						}
						else if($option_delta == 14){
							$option_delta = hexdec(substr($hex_message, $pos+2, 4))-269;
							$extra += 4;
						}
						$option_length = hexdec(substr($hex_message, $pos+1, 1));
						if($option_length == 13){
							$option_length = hexdec(substr($hex_message, $pos+2+$extra, 2))-13;
							$extra += 2;
						}
						else if($option_length == 14){
							$option_length = hexdec(substr($hex_message, $pos+2+$extra, 4))-269;
							$extra += 4;
						}
						$option_length *= 2;
					}
				}
			}
		}
		
		//	Add payload
		$payload = coap_library_get_payload_from_hex_message($hex_message);
		if($payload != null){
			$message->set_payload($payload);
		}
		
		return $message;
	}
	
}

class CoAPConstants{
	
	//	EXPONENTIAL BACKOFF
	public static $ACK_TIMEOUT = 2000; // 2 seconds
	public static $MAX_RETRANSMIT = 4;
	public static $RANDOM_FACTOR = 1.5;
	public static $NSTART = 1;
	public static $DEFAULT_LEISURE = 5000; // 5 seconds
	public static $PROBING_RATE = 5; // 1 Byte/second
	
	//	MAPPING
	public static $response_code_mapping = array(
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
	public static $format_mapping = array(
		0 => 'text/plain',
		40 => 'application/link-format',
		41 => 'application/xml',
		42 => 'application/octet-stream',
		47 => 'application/exi',
		50 => 'application/json'
	);
	
	public function ack_timeout(){
		return self::$ACK_TIMEOUT;
	}
	
	public function max_retransmit(){
		return self::$MAX_RETRANSMIT;
	}
	
	public function random_factor(){
		return self::$RANDOM_FACTOR;
	}
	
	public function nstart(){
		return self::$NSTART;
	}
	
	public function default_leisure(){
		return self::$DEFAULT_LEISURE;
	}
	
	public function probing_rate(){
		return self::$PROBING_RATE;
	}
	
	public function response_code_mapping(){
		return self::$response_code_mapping;
	}
	
	public function format_mapping(){
		return self::$format_mapping;
	}
	
}