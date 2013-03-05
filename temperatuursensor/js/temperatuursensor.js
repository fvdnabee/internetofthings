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

Drupal.Nodejs.callbacks.waardeOntvangen = {
  callback: function (message) {
    Drupal.nodejs_ajax.runCommands(message);
  }
};

})(jQuery);