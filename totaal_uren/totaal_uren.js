(function ($) {

var timerID;

$(document).ready(function() {
	$('#grafiekdata').hide();
	setTimeout(function(){google.load('visualization', '1', {'callback':'', 'packages':['corechart']})}, 10);
	google.setOnLoadCallback(drawChart);
	timerID = setInterval(function(){drawChart()}, 1000);
});

function drawChart() {
	var data_array = [['Datum', 'Kobe', 'Stef', 'Te behalen (625 uren)']];
	$('#grafiekdata > tbody > tr').each(function(){
		var row_array = [];
		var regex = /<td>(.*)-(.*)-(.*)<\/td><td>(.*)<\/td><td>(.*)<\/td>/;
		var matches = regex.exec($(this).html());
		if(matches){
			row_array.push(new Date(parseInt(matches[1]), parseInt(matches[2])-1, parseInt(matches[3]), 0, 0, 0).toString());
			row_array.push(parseFloat(matches[4]));
			row_array.push(parseFloat(matches[5]));
			row_array.push(parseFloat(625));
			data_array.push(row_array);
		}
	});
	var data = google.visualization.arrayToDataTable(data_array);

    var options = {
      'title': 'Verloop uren',
	  'legend': {'position': 'top', 'alignment': 'end'},
	  'height':400,
	  'max':700
    };

    var chart = new google.visualization.LineChart(document.getElementById('grafiek'));
    chart.draw(data, options);
	$("#grafiek").fadeIn("slow");
	clearInterval(timerID);
}

})(jQuery);