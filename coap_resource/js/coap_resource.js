(function ($) {

var polling;

$(document).ready(function() {
	setTimeout(function(){google.load('visualization', '1', {'callback':'', 'packages':['corechart']})}, 10);
	$('#polling_button').click(
		function(){
			console.log("polling button clicked, about to change interval to " + parseInt($('#polling_invoer').val())*1000 + ' milliseconds');
			clearInterval(polling);
			polling = setInterval(function(){
				console.log('start poll');
				$.ajax({
					type: "GET",
					url: "/coap_resource/poll",
					dataType: "text",
					success: tempReceived
				});
			}, parseInt(parseInt($('#polling_invoer').val())*1000));
			$.ajax({
				type: "POST",
				url: "/coap_resource/interval/" + $('#polling_invoer').val(),
				dataType: "text"
			});
		}
	);
	$('#polling_button').trigger('click');
	drawChart();
});

function tempReceived(html){
	var regex = /<error>(.*)<\/error><responded>(.*)<\/responded><get_response>(.*)<\/get_response><method>(.*)<\/method><response_type>(.*)<\/response_type><tr><td>(\d+)<\/td><td>(.*)<\/td><td>(.*)<\/td><td>(.*)<\/td><\/tr>/;
	var matches = regex.exec(html);
	console.log('input ajax call: ' + html);
	if(matches && matches[1] == 'none'){
		$('#error').hide();
		$('#errorimg').attr('style', 'visibility:hidden;');
		console.log('nieuwe hid: ' + matches[6] + ', oude hid: ' + $('#hid1').html());
		var nieuw = parseInt(matches[6]);
		var oud = parseInt($('#hid1').html());
		if(nieuw > oud){
			$('#error').html('');
			$('#historytable').hide();
			for (var i = 5; i >= 2; i--){
				var nr = i-1;
				$('#hid' + i).html($('#hid' + nr).html());
				$('#temperatuur' + i).html($('#temperatuur' + nr).html());
				$('#max_age' + i).html($('#max_age' + nr).html());
				$('#timestamp' + i).html($('#timestamp' + nr).html());
			}
			$('#hid1').html(matches[6]);
			$('#temperatuur1').html(matches[7]);
			$('#max_age1').html(matches[8]);
			$('#timestamp1').html(matches[9]);
			$('#historytable').fadeIn('slow');
			drawChart();
		}
		if(matches[2] == 1){
			$('#response').hide();
			$('#response_type').hide();
			$('#response').html("Response: " + matches[3]);
			$('#response_type').html("Response type: " + matches[5]);
			$('#response').fadeIn('slow');
			$('#response_type').fadeIn('slow');
		}
		drawChart();
	}
	else{
		var regex = /<error>(.*)<\/error><responded>(.*)<\/responded><get_response>(.*)<\/get_response><method>(.*)<\/method><response_type>(.*)<\/response_type>/;
		var matches = regex.exec(html);
		if(matches){
			if(matches[1] == 'unreachable'){
				if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/error.ico'){
					$('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/error.ico');
				}
				$('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
				$('#error').html('De sensor kon tijdens de laatste poging niet worden bereikt. Probeer later opnieuw.');
			}
			else if(matches[1] == 'delay'){
				if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/delay.gif'){
					$('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/delay.gif');
				}
				$('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
				$('#error').html('De sensor reageert trager dan normaal');
			}
			else if(matches[1] == 'broken'){
				if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/error.ico'){
					$('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/error.ico');
				}
				$('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
				$('#error').html('De sensor stopte tijdens de laatste poging met reageren. Probeer later opnieuw.');
			}
			else if(matches[1] == 'bad_uri'){
				if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/error.ico'){
					$('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/error.ico');
				}
				$('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
				$('#error').html('Er werd een ongeldige URI opgegeven');
			}
				if(matches[2] == '1'){
				$('#response').hide();
				$('#response_type').hide();
				$('#response').html("Response: " + matches[3]);
				$('#response_type').html("Response type: " + matches[5]);
				$('#response').fadeIn('slow');
				$('#response_type').fadeIn('slow');
			}
		}
		else{
			var regex = /<error>(.*)<\/error>/;
			var matches = regex.exec(html);
			if(matches){
				if(matches[1] == 'unreachable'){
					if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/error.ico'){
						$('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/error.ico');
					}
					$('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
					$('#error').html('De sensor kon tijdens de laatste poging niet worden bereikt. Probeer later opnieuw.');
				}
				else if(matches[1] == 'delay'){
					if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/delay.gif'){
						$('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/delay.gif');
					}
					$('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
					$('#error').html('De sensor reageert trager dan normaal');
				}
				else if(matches[1] == 'broken'){
					if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/error.ico'){
						$('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/error.ico');
					}
					$('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
					$('#error').html('De sensor stopte tijdens de laatste poging met reageren. Probeer later opnieuw.');
				}
				else if(matches[1] == 'bad_uri'){
					if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/error.ico'){
						$('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/error.ico');
					}
					$('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
					$('#error').html('Er werd een ongeldige URI opgegeven');
				}
			}
		}
	}
}

function drawChart() {
	var data = google.visualization.arrayToDataTable([
		['Timestamp', 'Temperatuur'],
		[parseInt($('#hid5').html()), parseFloat($('#temperatuur5').html())],
		[parseInt($('#hid4').html()), parseFloat($('#temperatuur4').html())],
		[parseInt($('#hid3').html()), parseFloat($('#temperatuur3').html())],
		[parseInt($('#hid2').html()), parseFloat($('#temperatuur2').html())],
		[parseInt($('#hid1').html()), parseFloat($('#temperatuur1').html())]
    ]);

    var options = {
      title: 'Geschiedenis temperatuur',
	  legend: {position: 'top', alignment: 'end'}
    };

    var chart = new google.visualization.LineChart(document.getElementById('grafiek'));
    chart.draw(data, options);
}

})(jQuery);