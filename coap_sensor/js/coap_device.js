(function ($) {

var counter = 0;

Drupal.behaviors.coap_resource = {
attach:	function(context) {
			setInterval(function(){ //every 2000 milliseconds, discoverReady is called
				$.ajax({
					type: "GET",
					url: Drupal.settings.basePath + "coap_device/poll/" + $('input[name|="nid"]').attr("value"),
					dataType: "text",
					success: discoverReady
				});
			}, 2000);
		}
};

//the only thing this funtion does is reload the page when the callback answers with 'reload'
function discoverReady(message){
	// console.log(message);
	if(message == 'reload'){
		location.reload();
	}
	else if(message == 'show_counter'){
		counter++;
		if(counter > 3 && counter < 7)
			$('#lbl_message').html('It\'s been ' + counter*2 + 's since the discovery has started. This is taking longer than usual.');
		if(counter > 6)
			$('#lbl_message').html('It\'s been ' + counter*2 + 's since the discovery has started. This is taking longer than usual. Please refresh the device.');
	}
}

})(jQuery);
