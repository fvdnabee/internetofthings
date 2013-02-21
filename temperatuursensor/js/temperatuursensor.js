$(document).ready(function(){
	$('#invoer').val('970013');
	$('#zoek').click();
});

$(document).on('click', '#zoek', function(){
	$("#block-temperatuursensor-user-custom").fadeOut("fast");
    $.ajax({
        type: "GET",
		url: "proxy.php?url=http://coap.me/coap://[2001:6a8:1d80:200::2]/obs",
        dataType: "html",
        success: parseHtml
     });
});

function parseHtml(html)
{
	$("#temperatuurblok").empty();
	 $("#temperatuurblok").append($(html).find("span").text());
  $("#block-temperatuursensor-user-custom").fadeIn("slow");
}