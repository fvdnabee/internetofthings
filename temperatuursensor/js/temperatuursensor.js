(function ($) {

$(document).ready(function(){
	$('#invoer').val('970013');
	$('#zoek').click();
	
	$('#zoek').click(function(){
		$("#block-temperatuursensor-user-custom").fadeOut("fast");
		$.ajax({
			type: "GET",
			url: "proxy.php?url=http://coap.me/coap://[2001:6a8:1d80:200::2]/obs",
			dataType: "html",
			success: parseHtml
		});
	});
});

function parseHtml(html)
{
	$("#temperatuurblok").empty();
	 $("#temperatuurblok").append($(html).find("span").text());
  $("#block-temperatuursensor-user-custom").fadeIn("slow");
}

// Drupal.Nodejs.callbacks.waardeOntvangen = {
  // callback: function (message) {
    // Drupal.nodejs_ajax.runCommands(message);
  // }
// };

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