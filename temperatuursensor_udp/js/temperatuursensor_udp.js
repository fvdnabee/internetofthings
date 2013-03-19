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
	var regex = /<tr><td>(\d+)<\/td><td>(.*)<\/td><td>(.*)<\/td><td>(.*)<\/td><\/tr>/;
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
			$('#temperatuur_udp' + i).html($('#temperatuur_udp' + nr).html());
			$('#max_age_udp' + i).html($('#max_age_udp' + nr).html());
			$('#timestamp_udp' + i).html($('#timestamp_udp' + nr).html());
		}
		$('#hid_udp1').html(matches[1]);
		$('#temperatuur_udp1').html(matches[2]);
		$('#max_age_udp1').html(matches[3]);
		$('#timestamp_udp1').html(matches[4]);
		$('#historytable_udp').fadeIn('slow');
	}
}

})(jQuery);