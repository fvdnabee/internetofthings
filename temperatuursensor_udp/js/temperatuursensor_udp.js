(function ($) {

Drupal.behaviors.temperatuursensor_udp = {
  attach: function(context) {
		var refreshId = setInterval(function(){
			$.ajax({
				type: "GET",
				url: "/observe/ajax",
				dataType: "text",
				success: tempReceived
			});
		}, 2000);
  }
};

function tempReceived(html){
	var regex = /<tr><td>(\d+)<\/td><td>(.*)<\/td><\/tr>/;
	var matches = regex.exec(html);
	console.log(html);
	console.log('nieuwe hid: ' + matches[1] + ', oude hid: ' + $('#hid_udp1').html());
	var nieuw = parseInt(matches[1]);
	var oud = parseInt($('#hid1').html());
	if(matches[1] > $('#hid_udp1').html()){
		$('#historytable_udp').hide();
		for (var i = 5; i >= 2; i--){
			var nr = i-1;
			$('#hid_udp' + i).html($('#hid_udp' + nr).html());
			$('#response_udp' + i).html($('#response_udp' + nr).html());
		}
		$('#hid_udp1').html(matches[1]);
		$('#response_udp1').html(matches[2]);
		$('#historytable_udp').fadeIn('slow');
	}
}

})(jQuery);