(function ($) {

$(document).ready(function(){
	$('#block-temperatuursensor-user-custom').append('<img id="loading" src="sites/all/modules/custom/temperatuursensor/images/ajax-loader.gif" />');
});

Drupal.behaviors.temperatuursensor = {
  attach: function(context) {
		var refreshId = setInterval(function(){
			$.ajax({
				type: "GET",
				url: "/temperatuur/ajax",
				dataType: "html",
				success: tempReceived
			});
		}, 2000);
  }
};

function tempReceived(html){
	var regex = /<tr><td>(\d+)<\/td><td>(.*)<\/td><td>(.*)<\/td><td>(.*)<\/td><\/tr>/;
	var matches = regex.exec(html);
	console.log('nieuwe hid: ' + matches[1] + ', oude hid: ' + $('#hid1').html());
	if(matches[1] > $('#hid1').html()){
		$("#loading").fadeOut('slow');
		$('#historytable').hide();
		$('#temperatuur').html(matches[2]);
		$('#max_age').html(matches[3]);
		for (var i = 5; i >= 2; i--){
			var nr = i-1;
			$('#hid' + i).html($('#hid' + nr).html());
			$('#temperature' + i).html($('#temperature' + nr).html());
			$('#max_age' + i).html($('#max_age' + nr).html());
			$('#timestamp' + i).html($('#timestamp' + nr).html());
		}
		$('#hid1').html(matches[1]);
		$('#temperature1').html(matches[2]);
		$('#max_age1').html(matches[3]);
		$('#timestamp1').html(matches[4]);
		$('#historytable').fadeIn('slow');
	}
}

})(jQuery);