(function ($) {

var polling;

var get_pending = false;
var put_pending = false;
var post_pending = false;
var delete_pending = false;
var observe_pending = false;

$(document).ready(function() {
	setTimeout(function(){google.load('visualization', '1', {'callback':'', 'packages':['corechart']})}, 20);
	$('.POLLING_BUTTON').click(
		function(){
			var uri = $($(this).parent().parent().find('.uri')[0]).html();
			var milliseconds = parseInt($($(this).parent().find('.POLLING_INVOER')[0]).val())*1000;
			console.log("polling button clicked, about to change interval to " + milliseconds + ' milliseconds for ' + uri);
			clearInterval(polling);
			polling = setInterval(function(){
				console.log('start poll for ' + uri);
				$.ajax({
					type: "GET",
					url: Drupal.settings.basePath + "coap_resource/poll/" + uri.replace(/\//g, '|'),
					dataType: "text",
					success: valueReceived
				});
			}, milliseconds);
			$.ajax({
				type: "POST",
				url: Drupal.settings.basePath + "coap_resource/interval/" + uri.replace(/\//g, '|') + "/" + parseInt($($(this).parent().find('.POLLING_INVOER')[0]).val()),
				dataType: "text"
			});
			
		}
	);
	
	$('.REQUEST_BUTTON').click(
		function(){
			var uri = $($(this).parent().parent().find('.uri')[0]).html();
			var method = $(this).parent().attr('class');
			var input_element = $(this).parent().find('.' + method + "_INPUT")[0];
			var input = '';
			if(typeof(input_element) != "undefined"){
				input = $(input_element).val();
			}
			console.log("Button clicked: starting " + method + " request for " + uri + " with input = " + input);
			var label_id = "#lbl_" + method + "_" + uri;
			label_id = label_id.replace(/\:/g, '\\\:');
			label_id = label_id.replace(/\//g, '\\\/');
			$(label_id).hide();
			$(label_id).html("Fetching response...");
			$(label_id).fadeIn("slow");
			$.ajax({
				type: method,
				url: Drupal.settings.basePath + "coap_resource/request/" + method + "/" + uri.replace(/\//g, '|') + "/" + input,
				dataType: "text",
				success: getResponse
			});
			switch(method){
				case "GET":
					get_pending = true;
					break;
				case "PUT":
					put_pending = true;
					break;
				case "POST":
					post_pending = true;
					break;
				case "DELETE":
					delete_pending = true;
					break;
			}
			$(this).parent().find('.img_status').attr("src", "");
			$(this).parent().find('.img_status').attr("style", "display: none");
		}
	);
	
	$('.OBSERVE_BUTTON').click(
		function(){
			var uri = $($(this).parent().parent().find('.uri')[0]).html();
			if($(this).val() == "Start Observing"){
				console.log("Button clicked: starting observe for " + uri);
				$(this).val('Starting Observe...');
				$.ajax({
					type: "GET",
					url: Drupal.settings.basePath + "coap_resource/observe/" + uri.replace(/\//g, '|') + "/start",
					dataType: "text",
					success: observeResponse
				});
				observe_pending = true;
			}
			else if($(this).val() == "Stop Observing"){
				console.log("Button clicked: stopping observe for " + uri);
				$(this).val('Start Observing');
				$.ajax({
					type: "GET",
					url: Drupal.settings.basePath + "coap_resource/observe/" + uri.replace(/\//g, '|') + "/stop"
				});
			}
		}
	);
	
	$('.historyselect').change(
		function(){
			var chosen_nr = parseInt($(this).find('option:selected').val());
			var current_nr = parseInt($(this).parent().find('.historytable > tbody > tr:visible').length);
			if(chosen_nr > current_nr){
				for(var i = (current_nr+1); i < (chosen_nr+1); i++){
					$(this).parent().find('.row' + i).fadeIn('slow');
				}
			}
			else if(chosen_nr < current_nr){
				for(var i = current_nr; i > chosen_nr; i--){
					$(this).parent().find('.row' + i).fadeOut("slow");
				}
			}
		}
	);
	
	$('.graphselect').change(
		function(){
			drawChart($(this).parent().parent().attr("id"));
		}
	);
	
	$('.POLLING_BUTTON').trigger('click');
});

function valueReceived(html){
	var xmlDoc = $.parseXML(html.toString());
	var $xml = $(xmlDoc);
	var uri = $($xml.find('uri')).text();
	console.log('input from poll for ' + uri + ': ' + html);
	var entrys = $($xml.find('entrys'));
	$(entrys.children("entry").get().reverse()).each(
		function(){
			var id = '#' + uri;
			var observe_label = "#lbl_OBSERVE_" + uri;
			observe_label = observe_label.replace(/\:/g, '\\\:');
			observe_label = observe_label.replace(/\//g, '\\\/');
			$(observe_label).html($(this).find("value").text());
			id = id.replace(/\:/g, '\\\:');
			id = id.replace(/\//g, '\\\/');
			var table = $(id).find(".historytable");
			table.find('.row1').hide();
			for(var i = 10; i > 1; i--){
				var old_index = i-1;
				table.find('.row' + i).html(table.find('.row' + old_index).html());
			}
			table.find('.row1').html("<td>" + $(this).find("timestamp").text() + "</td><td>" + $(this).find("value").text() + "</td>");
			table.find('.row1').fadeIn('slow');
		}
	);
	
	var error_str = $($xml.find('error')).text();
	resource_div = '#' + uri;
	resource_div = resource_div.replace(/\:/g, '\\\:');
	resource_div = resource_div.replace(/\//g, '\\\/');
	var img_status = $(resource_div).find('.img_status');
	var src;
	if(error_str == "none"){
		src = "";
	}
	else if(error_str == "delay" && img_status.attr("src") != Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/delay.gif"){
		src = Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/delay.gif";
	}
	else if((error_str == "broken" || error_str == "unreachable") && img_status.attr("src") != Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/error.ico"){
		src = Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/error.ico";
	}
	if(get_pending){
		$(resource_div + ' > .' + 'GET > .img_status').attr("src", src);
		if(src == "" && $(resource_div + ' > .' + 'GET > .img_status').attr("src") != Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/valid.ico"){
			$(resource_div + ' > .' + 'GET > .img_status').attr("style", "display: none");
		}
		else{
			$(resource_div + ' > .' + 'GET > .img_status').attr("style", "display: inline");
		}
	}
	if(put_pending){
		$(resource_div + ' > .' + 'PUT > .img_status').attr("src", src);
		if(src == ""){
			$(resource_div + ' > .' + 'PUT > .img_status').attr("style", "display: none");
		}
		else{
			$(resource_div + ' > .' + 'PUT > .img_status').attr("style", "display: inline");
		}
	}
	if(post_pending){
		$(resource_div + ' > .' + 'POST > .img_status').attr("src", src);
		if(src == ""){
			$(resource_div + ' > .' + 'POST > .img_status').attr("style", "display: none");
		}
		else{
			$(resource_div + ' > .' + 'POST > .img_status').attr("style", "display: inline");
		}
	}
	if(delete_pending){
		$(resource_div + ' > .' + 'DELETE > .img_status').attr("src", src);
		if(src == ""){
			$(resource_div + ' > .' + 'DELETE > .img_status').attr("style", "display: none");
		}
		else{
			$(resource_div + ' > .' + 'DELETE > .img_status').attr("style", "display: inline");
		}
	}
	if(observe_pending){
		$(resource_div + ' > .' + 'OBSERVE > .img_status').attr("src", src);
		if(src == ""){
			$(resource_div + ' > .' + 'OBSERVE > .img_status').attr("style", "display: none");
		}
		else{
			$(resource_div + ' > .' + 'OBSERVE > .img_status').attr("style", "display: inline");
		}
	}
	
	drawChart(uri);
}

function discoverReady(xml){
	//omdat we overhevelen naar jquery zal dit interval niet meer gestopt mogen worden, aangezien de pagina niet meer herladen zal worden, moet de jquery code blijven lopen
	console.log(xml);
	if(xml == 'reload'){
		location.reload();
	}
}

function drawChart(uri) {
	var data_array = [['Nr', 'Value']];
	
	var resource_div = '#' + uri;
	resource_div = resource_div.replace(/\:/g, '\\\:');
	resource_div = resource_div.replace(/\//g, '\\\/');
	resource_div = $(resource_div);
	var amount = resource_div.find('.history > .historyselect > option:selected').val();
	var type = resource_div.find(".graphselect > option:selected").val();
	if(type == "Pie"){
		for(var i = amount; i > 0; i--){
			data_array.push([(amount-i+1).toString(), parseFloat(resource_div.find('.row' + i + ' > .coap_value').html())]);
		}
	}
	else{
		for(var i = amount; i > 0; i--){
			data_array.push([amount-i+1, parseFloat(resource_div.find('.row' + i + ' > .coap_value').html())]);
		}
	}
	var data = google.visualization.arrayToDataTable(data_array);

    var options = {
      title: 'History of fetched values',
	  legend: {position: 'top', alignment: 'end'}
    };

	if(type == "Line"){
		var chart = new google.visualization.LineChart(document.getElementById("div_graphimage_" + uri));
	}
	else if(type == "Pie"){
		var chart = new google.visualization.PieChart(document.getElementById("div_graphimage_" + uri));
	}
	else if(type == "Column"){
		var chart = new google.visualization.ColumnChart(document.getElementById("div_graphimage_" + uri));
	}
	if(type != "None"){
		chart.draw(data, options);
	}
	else{
		resource_div.find(".graphimage").empty();
	}
}

function getResponse(input){
	console.log("Response for GET request: " + input);
	var regex = /<uri>(.*)<\/uri><method>(.*)<\/method><response>(.*)<\/response><code>(.*)<\/code>/;
	var matches = regex.exec(input);
	if(matches){
		console.log('match');
		var label = '#lbl_' + matches[2] + "_" + matches[1];
		label = label.replace(/\:/g, '\\\:');
		label = label.replace(/\//g, '\\\/');
		$(label).hide();
		if(matches[4] == "133"){
			$(label).html("This method is not supported on this resource");
		}
		else{
			$(label).html(matches[3]);
		}
		$(label).fadeIn('slow');
		resource_div = '#' + matches[1];
		resource_div = resource_div.replace(/\:/g, '\\\:');
		resource_div = resource_div.replace(/\//g, '\\\/');
		var img_status = $(resource_div + " > ." + matches[2] + " > .img_status");
		if(matches[4] == "133"){
			img_status.attr("src", Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/error.ico");
		}
		else{
			img_status.attr("src", Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/valid.ico");
		}
		img_status.attr("style", "display: inline");
		switch(matches[2]){
			case "GET":
				get_pending = false;
				break;
			case "PUT":
				put_pending = false;
				break;
			case "POST":
				post_pending = false;
				break;
			case "DELETE":
				delete_pending = false;
				break;
		}
	}
}

function observeResponse(input){
	console.log("Response for observe request: " + input);
	var regex = /<uri>(.*)<\/uri><response>(.*)<\/response>/;
	var matches = regex.exec(input);
	if(matches){
		if(matches[2] == "success"){
			var button = '#btn_OBSERVE_' + matches[1];
			button = button.replace(/\:/g, '\\\:');
			button = button.replace(/\//g, '\\\/');
			$(button).val("Stop Observing");
		}
		else if(matches[2] == "failed"){
			var button = '#btn_OBSERVE_' + matches[1];
			button = button.replace(/\:/g, '\\\:');
			button = button.replace(/\//g, '\\\/');
			$(button).val("Start Observing");
			var label = '#lbl_OBSERVE_' + matches[1];
			label = label.replace(/\:/g, '\\\:');
			label = label.replace(/\//g, '\\\/');
			$(label).html("Observe could not be started");
		}
	}
}

})(jQuery);
