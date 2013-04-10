(function ($) {

// $(document).ready(function() {
	// setTimeout(function(){google.load('visualization', '1', {'callback':'', 'packages':['corechart']})}, 10);
	// google.setOnLoadCallback(drawChart);
	// setTimeout(function(){drawChart()}, 1000);
// });

// Drupal.behaviors.sensor = {
  // attach: 	function(context) {
				// var refreshId = setInterval(function(){
					// timesRun += 1;
					// if(timesRun === 60){
						// clearInterval(interval);
					// }
					// $.ajax({
						// type: "GET",
						// url: "/sensor/poll",
						// dataType: "text",
						// success: discoverReady
					// });
				// }, 2000);
			// }
// };

var refreshId;

Drupal.behaviors.temperatuursensor_udp = {
  attach: 	function(context) {
				refreshId = setInterval(function(){
					$.ajax({
						type: "GET",
						url: "/sensor/poll",
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
	// else{
		// $('#edit-tableselect-container').empty();
		// var table = '';
		// table += '<table class="sticky-header" style="position: fixed; top: 30px; left: 407px; visibility: hidden;"><thead style=""><tr><th>link name</th><th>human-readable name</th> </tr></thead></table><table class="sticky-enabled tableheader-processed sticky-table">';
		// table += '<thead><tr><th>link name</th><th>human-readable name</th> </tr></thead>';
		// table += '<tbody><tr class="odd"><td colspan="2" class="empty message">Enter a URI and click on \'Reload device\' to display resources here.</td> </tr>'
		// table += '</tbody></table>';
		// $('#edit-tableselect-container').append(table);
	// }
	
	 
	 //console.log('input ajax call: ' + html);
	// if(matches && matches[1] == 'none'){
		// $('#error').hide();
		// $('#errorimg').attr('style', 'visibility:hidden;');
		// console.log('nieuwe hid: ' + matches[4] + ', oude hid: ' + $('#hid1').html());
		// var nieuw = parseInt(matches[4]);
		// var oud = parseInt($('#hid1').html());
		// if(nieuw > oud){
			// $('#error').html('');
			// $('#historytable').hide();
			// for (var i = 5; i >= 2; i--){
				// var nr = i-1;
				// $('#hid' + i).html($('#hid' + nr).html());
				// $('#temperatuur' + i).html($('#temperatuur' + nr).html());
				// $('#max_age' + i).html($('#max_age' + nr).html());
				// $('#timestamp' + i).html($('#timestamp' + nr).html());
			// }
			// $('#hid1').html(matches[4]);
			// $('#temperatuur1').html(matches[5]);
			// $('#max_age1').html(matches[6]);
			// $('#timestamp1').html(matches[7]);
			// $('#historytable').fadeIn('slow');
			// drawChart();
		// }
		// if(matches[2] == 'yes'){
			// $('#get_response').hide();
			// $('#get_response').html("Response: " + matches[3]);
			// $('#get_response').fadeIn('slow');
		// }
	// }
	// else{
		// var regex = /<error>(.*)<\/error><responded>(.*)<\/responded><get_response>(.*)<\/get_response>/;
		// var matches = regex.exec(html);
		// if(matches){
			// if(matches[1] == 'unreachable'){
				// if($('#errorimg').attr('src') != 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico'){
					// $('#errorimg').attr('src', 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico');
				// }
				// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
				// $('#error').html('De sensor kon niet worden bereikt');
			// }
			// else if(matches[1] == 'delay'){
				// if($('#errorimg').attr('src') != 'sites/all/modules/custom/temperatuursensor_udp/images/delay.gif'){
					// $('#errorimg').attr('src', 'sites/all/modules/custom/temperatuursensor_udp/images/delay.gif');
				// }
				// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
				// $('#error').html('De sensor reageert trager dan normaal');
			// }
			// else if(matches[1] == 'broken'){
				// if($('#errorimg').attr('src') != 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico'){
					// $('#errorimg').attr('src', 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico');
				// }
				// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
				// $('#error').html('De sensor reageert niet meer');
			// }
			// else if(matches[1] == 'bad_uri'){
				// if($('#errorimg').attr('src') != 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico'){
					// $('#errorimg').attr('src', 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico');
				// }
				// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
				// $('#error').html('Er werd een ongeldige URI opgegeven');
			// }
				// if(matches[2] == 'yes'){
				// $('#get_response').hide();
				// $('#get_response').html("Response: " + matches[3]);
				// $('#get_response').fadeIn('slow');
			// }
		// }
		// else{
			// var regex = /<error>(.*)<\/error>/;
			// var matches = regex.exec(html);
			// if(matches){
				// if(matches[1] == 'unreachable'){
					// if($('#errorimg').attr('src') != 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico'){
						// $('#errorimg').attr('src', 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico');
					// }
					// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
					// $('#error').html('De sensor kon niet worden bereikt');
				// }
				// else if(matches[1] == 'delay'){
					// if($('#errorimg').attr('src') != 'sites/all/modules/custom/temperatuursensor_udp/images/delay.gif'){
						// $('#errorimg').attr('src', 'sites/all/modules/custom/temperatuursensor_udp/images/delay.gif');
					// }
					// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
					// $('#error').html('De sensor reageert trager dan normaal');
				// }
				// else if(matches[1] == 'broken'){
					// if($('#errorimg').attr('src') != 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico'){
						// $('#errorimg').attr('src', 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico');
					// }
					// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
					// $('#error').html('De sensor reageert niet meer');
				// }
				// else if(matches[1] == 'bad_uri'){
					// if($('#errorimg').attr('src') != 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico'){
						// $('#errorimg').attr('src', 'sites/all/modules/custom/temperatuursensor_udp/images/error.ico');
					// }
					// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
					// $('#error').html('Er werd een ongeldige URI opgegeven');
				// }
			// }
		// }
	// }
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