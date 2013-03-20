(function ($) {

$(document).ready(function() {
	setTimeout(function(){google.load('visualization', '1', {'callback':'', 'packages':['corechart']})}, 10);
	google.setOnLoadCallback(drawChart);
	setTimeout(function(){drawChart()}, 1000);
});

Drupal.behaviors.temperatuursensor_udp = {
  attach: 	function(context) {
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
	var regex = /<error>(.*)<\/error><responded>(.*)<\/responded><get_response>(.*)<\/get_response><tr><td>(\d+)<\/td><td>(.*)<\/td><td>(.*)<\/td><td>(.*)<\/td><\/tr>/;
	var matches = regex.exec(html);
	console.log('input ajax call: ' + html);
	if(matches[1] == 'none'){
		console.log('nieuwe hid: ' + matches[4] + ', oude hid: ' + $('#hid_udp1').html());
		var nieuw = parseInt(matches[4]);
		var oud = parseInt($('#hid1').html());
		if(matches[4] > $('#hid_udp1').html()){
			$("#edit-submit").attr("value", "Stop met observen");
			$('#errorimg').attr('style', 'visibility:hidden;');
			$('#error').html('');
			$('#historytable_udp').hide();
			for (var i = 5; i >= 2; i--){
				var nr = i-1;
				$('#hid_udp' + i).html($('#hid_udp' + nr).html());
				$('#temperatuur_udp' + i).html($('#temperatuur_udp' + nr).html());
				$('#max_age_udp' + i).html($('#max_age_udp' + nr).html());
				$('#timestamp_udp' + i).html($('#timestamp_udp' + nr).html());
			}
			$('#hid_udp1').html(matches[4]);
			$('#temperatuur_udp1').html(matches[5]);
			$('#max_age_udp1').html(matches[6]);
			$('#timestamp_udp1').html(matches[7]);
			$('#historytable_udp').fadeIn('slow');
			drawChart();
		}
		if(matches[2] == 'yes'){
			$('#get_response').hide();
			$('#get_response').html("Response: " + matches[3]);
			$('#get_response').fadeIn('slow');
		}
	}
	else{
		if(matches[1] == 'unreachable'){
			if($('#errorimg').attr('src') != 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico'){
				$('#errorimg').attr('src', 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico');
			}
			$('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
			$('#error').html('De sensor kon niet worden bereikt');
		}
		else if(matches[1] == 'delay'){
			if($('#errorimg').attr('src') != 'sites/all/modules/custom/temperatuursensor_udp/images/delay.gif'){
				$('#errorimg').attr('src', 'sites/all/modules/custom/temperatuursensor_udp/images/delay.gif');
			}
			$('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
			$('#error').html('De sensor reageert trager dan normaal');
		}
		else if(matches[1] == 'broken'){
			if($('#errorimg').attr('src') != 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico'){
				$('#errorimg').attr('src', 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico');
			}
			$('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
			$('#error').html('De sensor reageert niet meer');
		}
		$("#edit-submit").attr("value", "Start met observen");
	}
}

function drawChart() {
	var data = google.visualization.arrayToDataTable([
		['Timestamp', 'Temperatuur'],
		[parseInt($('#hid_udp5').html()), parseFloat($('#temperatuur_udp5').html())],
		[parseInt($('#hid_udp4').html()), parseFloat($('#temperatuur_udp4').html())],
		[parseInt($('#hid_udp3').html()), parseFloat($('#temperatuur_udp3').html())],
		[parseInt($('#hid_udp2').html()), parseFloat($('#temperatuur_udp2').html())],
		[parseInt($('#hid_udp1').html()), parseFloat($('#temperatuur_udp1').html())]
    ]);

    var options = {
      title: 'Geschiedenis temperatuur',
	  legend: {position: 'top', alignment: 'end'}
    };

    var chart = new google.visualization.LineChart(document.getElementById('grafiek'));
    chart.draw(data, options);
}

})(jQuery);