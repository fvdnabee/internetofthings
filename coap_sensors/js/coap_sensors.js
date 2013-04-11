(function ($) {

// $(document).ready(function() {
	// setTimeout(function(){google.load('visualization', '1', {'callback':'', 'packages':['corechart']})}, 10);
	// google.setOnLoadCallback(drawChart);
	// setTimeout(function(){drawChart()}, 1000);
// });

// Drupal.behaviors.temperatuursensor_udp = {
  // attach: 	function(context) {
				// var refreshId = setInterval(function(){
					// $.ajax({
						// type: "GET",
						// url: "/coap_sensors/observe",
						// dataType: "text",
						// success: tempReceived
					// });
				// }, 2000);
			// }
			
// };

var refreshId;

Drupal.behaviors.coap_sensors = {
  attach: 	function(context) {
				refreshId = setInterval(function(){
					$.ajax({
						type: "GET",
						url: "/coap_sensors/discover",
						dataType: "text",
						success: discoverReady
					});
				}, 2000);
			}
			
};

function discoverReady(xml){
	clearInterval(refreshId);
	//console.log('jquery opgestart');
	//console.log(xml);
	
	if(xml.length != 0){
		$('#edit-tableselect-container').empty();
		
		var table = '<table class="sticky-header" style="position: fixed; top: 30px; left: 407px; visibility: hidden;"><thead style=""><tr><th></th><th>link name</th><th>human-readable name</th> </tr></thead></table><table class="sticky-enabled tableheader-processed sticky-table">';	
		table += '<thead><tr><th></th><th>link name</th><th>human-readable name</th> </tr></thead>';
		table += '<tbody>';
		var links = xml.match(/<link>(.+?)<\/link>/g);
		for(var i=0 ; i< links.length ; i++){
			var link_attr = /<link><link_name>(.*?)<\/link_name><title>(.*?)<\/title><\/link>/.exec(links[i]);
			if(i%2 == 0){
				table += '<tr class="even"><td><div class="form-item form-type-radio form-item-table">' +
					'<input type="radio" id="edit-table-"' + i + '" name="table" value="' + i + '" class="form-radio">' + 
					'</div></td><td>' + link_attr[1] + '</td><td>' + link_attr[2] + '</td> </tr>';
			}
			else{
				table += '<tr class="odd"><td><div class="form-item form-type-radio form-item-table">' +
					'<input type="radio" id="edit-table-"' + i + '" name="table" value="' + i + '" class="form-radio">' + 
					'</div></td><td>' + link_attr[1] + '</td><td>' + link_attr[2] + '</td> </tr>';
			}
		}	
		table += '</tbody>';
		table += '</table>';
		$('#edit-tableselect-container').append(table);
	}
}

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
	}
	else{
		var regex = /<error>(.*)<\/error><responded>(.*)<\/responded><get_response>(.*)<\/get_response><method>(.*)<\/method><response_type>(.*)<\/response_type>/;
		var matches = regex.exec(html);
		if(matches){
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
			else if(matches[1] == 'bad_uri'){
				if($('#errorimg').attr('src') != 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico'){
					$('#errorimg').attr('src', 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico');
				}
				$('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
				$('#error').html('Er werd een ongeldige URI opgegeven');
			}
				if(matches[2] == 'yes'){
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
				else if(matches[1] == 'bad_uri'){
					if($('#errorimg').attr('src') != 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico'){
						$('#errorimg').attr('src', 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico');
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