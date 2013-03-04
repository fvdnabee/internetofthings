$(document).ready(function(){
	//$('#invoer').val('970013');
	//$('#zoek').click();
});

$(document).on('click', '#zoek', function(){
	$("#block-weather-info-user-custom").fadeOut("fast");
    $.ajax({
        type: "GET",
        url: "proxy.php?url=http%3A%2F%2Fweather.yahooapis.com%2Fforecastrss%3Fw%3D" + $("#invoer").val() + "%26u%3Dc",
		//url: "proxy.php?url=http%3A%2F%2Fcoap.me%2Fcoap%3A%2F%2F[2001%3A6a8%3A1d80%3A200%3A%3A2]%2Fobs,
		//url: "proxy.php?url=http://coap.me/coap://[2001:6a8:1d80:200::2]/obs",
        dataType: "xml",
        success: parseXml
     });
});

function parseXml(xml)
{
	$("#weerblok").empty();
	 $("#weerblok").append($(xml).find("description").text());
  $("#block-weather-info-user-custom").fadeIn("slow");
}