(function ($) {

var timerID;

$(document).ready(function() {
	setTimeout(function(){google.load('visualization', '1', {'callback':'', 'packages':['corechart']})}, 10);
	//google.setOnLoadCallback(drawChart);
	console.log("start ajax call");
	$.ajax({
		type: "GET",
		url: "/totaal_uren/grafiekdata",
		dataType: "text",
		success: dataReceived
	});
});

function drawChart() {
	console.log("begin grafiek tekenen");
	var data_array = [['Datum', 'Kobe', 'Stef', 'Doel (625 uren)']];
	$('#grafiekdata > tbody > tr').each(function(){
		var row_array = [];
		var regex = /<td>(.*)-(.*)-(.*)<\/td><td>(.*)<\/td><td>(.*)<\/td>/;
		var matches = regex.exec($(this).html());
		if(matches){
			var date = new Date(parseInt(matches[1]), parseInt(matches[2])-1, parseInt(matches[3]), 0, 0, 0);
			var day = date.getDate();
			var month = 1 + date.getMonth();
			var year = date.getFullYear();
			row_array.push(day.toString() + '/' + month + '/' + year);
			row_array.push(parseFloat(matches[4]));
			row_array.push(parseFloat(matches[5]));
			row_array.push(parseFloat(625));
			data_array.push(row_array);
		}
	});
	var data = google.visualization.arrayToDataTable(data_array);
	
	var options = {
      'title': 'Verloop uren',
	  // 'legend': {'position': 'top', 'alignment': 'end'},
	  'height':500,
	  'width':parseInt($('#header').width())
	  // 'max':700
    };
	
    var chart = new google.visualization.LineChart(document.getElementById('grafiekafbeelding'));
    chart.draw(data, options);
	$('#melding').hide();
	$('#waiting').hide();
	$("#grafiekafbeelding").fadeIn("slow");
}

function dataReceived(data){
	console.log("data ontvangen");
	$('#grafiek').append(data);
	$('#grafiekafbeelding').hide();
	drawChart();
}

})(jQuery);