(function ($) {

var polling;
var refreshId;

var get_pending = false;
var put_pending = false;
var post_pending = false;
var delete_pending = false;
var observe_pending = false;

Drupal.behaviors.coap_resource = {
  attach: 	function(context) {
				refreshId = setInterval(function(){
					$.ajax({
						type: "GET",
						url: "/coap_resource/discovery/" + $('input[name|="nid"]').attr("value"),
						dataType: "text",
						success: discoverReady
					});
				}, 2000);
			}
			
};

$(document).ready(function() {
	setTimeout(function(){google.load('visualization', '1', {'callback':'', 'packages':['corechart']})}, 10);
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
					url: "/coap_resource/poll/" + uri.replace(/\//g, '|'),
					dataType: "text",
					success: valueReceived
				});
			}, milliseconds);
			$.ajax({
				type: "POST",
				url: "/coap_resource/interval/" + uri.replace(/\//g, '|') + "/" + parseInt($($(this).parent().find('.POLLING_INVOER')[0]).val()),
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
				url: "/coap_resource/request/" + method + "/" + uri.replace(/\//g, '|') + "/" + input,
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
					url: "/coap_resource/observe/" + uri.replace(/\//g, '|') + "/start",
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
					url: "/coap_resource/observe/" + uri.replace(/\//g, '|') + "/stop"
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
	
	$('.POLLING_BUTTON').trigger('click');
	
	drawChart();
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
	if(error_str == "none" /*&& img_status.attr("src") != Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/valid.ico"*/){
		src = "";
		// img_status.attr("src", "");
		// img_status.attr("style", "display: none");
	}
	else if(error_str == "delay" && img_status.attr("src") != Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/delay.gif"){
		src = Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/delay.gif";
		// img_status.attr("src", Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/delay.gif");
		// img_status.attr("style", "display: inline");
	}
	else if((error_str == "broken" || error_str == "unreachable") && img_status.attr("src") != Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/error.ico"){
		src = Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/error.ico";
		// img_status.attr("src", Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/error.ico");
		// img_status.attr("style", "display: inline");
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
	
	
	
	// var regex = /<error>(.*)<\/error><responded>(.*)<\/responded><get_response>(.*)<\/get_response><method>(.*)<\/method><response_type>(.*)<\/response_type><tr><td>(\d+)<\/td><td>(.*)<\/td><td>(.*)<\/td><td>(.*)<\/td><\/tr>/;
	// var matches = regex.exec(html);
	// console.log('input ajax call: ' + html);
	// if(matches && matches[1] == 'none'){
		// $('#error').hide();
		// $('#errorimg').attr('style', 'visibility:hidden;');
		// console.log('nieuwe hid: ' + matches[6] + ', oude hid: ' + $('#hid1').html());
		// var nieuw = parseInt(matches[6]);
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
			// $('#hid1').html(matches[6]);
			// $('#temperatuur1').html(matches[7]);
			// $('#max_age1').html(matches[8]);
			// $('#timestamp1').html(matches[9]);
			// $('#historytable').fadeIn('slow');
			// drawChart();
		// }
		// if(matches[2] == 1){
			// $('#response').hide();
			// $('#response_type').hide();
			// $('#response').html("Response: " + matches[3]);
			// $('#response_type').html("Response type: " + matches[5]);
			// $('#response').fadeIn('slow');
			// $('#response_type').fadeIn('slow');
		// }
		// drawChart();
	// }
	// else{
		// var regex = /<error>(.*)<\/error><responded>(.*)<\/responded><get_response>(.*)<\/get_response><method>(.*)<\/method><response_type>(.*)<\/response_type>/;
		// var matches = regex.exec(html);
		// if(matches){
			// if(matches[1] == 'unreachable'){
				// if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/error.ico'){
					// $('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/error.ico');
				// }
				// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
				// $('#error').html('De sensor kon tijdens de laatste poging niet worden bereikt. Probeer later opnieuw.');
			// }
			// else if(matches[1] == 'delay'){
				// if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/delay.gif'){
					// $('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/delay.gif');
				// }
				// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
				// $('#error').html('De sensor reageert trager dan normaal');
			// }
			// else if(matches[1] == 'broken'){
				// if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/error.ico'){
					// $('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/error.ico');
				// }
				// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
				// $('#error').html('De sensor stopte tijdens de laatste poging met reageren. Probeer later opnieuw.');
			// }
			// else if(matches[1] == 'bad_uri'){
				// if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/error.ico'){
					// $('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/error.ico');
				// }
				// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
				// $('#error').html('Er werd een ongeldige URI opgegeven');
			// }
				// if(matches[2] == '1'){
				// $('#response').hide();
				// $('#response_type').hide();
				// $('#response').html("Response: " + matches[3]);
				// $('#response_type').html("Response type: " + matches[5]);
				// $('#response').fadeIn('slow');
				// $('#response_type').fadeIn('slow');
			// }
		// }
		// else{
			// var regex = /<error>(.*)<\/error>/;
			// var matches = regex.exec(html);
			// if(matches){
				// if(matches[1] == 'unreachable'){
					// if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/error.ico'){
						// $('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/error.ico');
					// }
					// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
					// $('#error').html('De sensor kon tijdens de laatste poging niet worden bereikt. Probeer later opnieuw.');
				// }
				// else if(matches[1] == 'delay'){
					// if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/delay.gif'){
						// $('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/delay.gif');
					// }
					// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
					// $('#error').html('De sensor reageert trager dan normaal');
				// }
				// else if(matches[1] == 'broken'){
					// if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/error.ico'){
						// $('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/error.ico');
					// }
					// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
					// $('#error').html('De sensor stopte tijdens de laatste poging met reageren. Probeer later opnieuw.');
				// }
				// else if(matches[1] == 'bad_uri'){
					// if($('#errorimg').attr('src') != 'sites/all/modules/custom/coap_resource/images/error.ico'){
						// $('#errorimg').attr('src', 'sites/all/modules/custom/coap_resource/images/error.ico');
					// }
					// $('#errorimg').attr('style', 'visibility:visible;width:25px;height:25px;');
					// $('#error').html('Er werd een ongeldige URI opgegeven');
				// }
			// }
		// }
	// }
}

function discoverReady(xml){
	console.log(xml);
	if(xml == '::new=1'){
		location.reload();
	}
	//omdat we overhevelen naar jquery zal dit interval niet meer gestopt mogen worden, aangezien de pagina niet meer herladen zal worden, moet de jquery code blijven lopen
	// if(xml == 'reset'){
		// console.log(xml);
		// clearInterval(refreshId);
	// }
	// else{
		// console.log(xml);
	// }
	
	//console.log('jquery opgestart');
	
	// if(xml.length != 0){
		// $('#edit-tableselect-container').empty();	
		// var table = '';
		// var resources = xml.match(/<resource>(.+?)<\/resource>/g);
		// if(resources){ //<input type="checkbox" class="form-checkbox" title="Select all rows in this table"> ==> stukje in eerste <th> dat ik weggelaten heb
			// table += '<table class="sticky-enabled table-select-processed tableheader-processed sticky-table"><thead><tr><th class="select-all"></th><th>uri</th><th>human-readable name</th> </tr></thead>';	
			// table += '<tbody>';
			// for(var i=0 ; i< resources.length ; i++){
				// var resource_attr = /<resource><uri>(.*?)<\/uri><title>(.*?)<\/title><watch>(.*?)<\/watch><\/resource>/.exec(resources[i]);
				// if(i%2 == 0){
					// table += '<tr class="even">';
				// }
				// else{
					// table += '<tr class="odd">';
				// }
				// table += '<td><div class="form-item form-type-checkbox form-item-table-' + i + '">' +
						 // '<input type="checkbox" id="edit-table-' + i + '" name="table[' + i + ']" value="' + i + '" class="form-checkbox";';
				// if(resource_attr[3] == 1) table += ' checked="yes"';
				// table += '></div></td><td>' + resource_attr[1] + '</td><td>' + resource_attr[2] + '</td> </tr>';
			// }
		// }
		// else{
			// table += '<table class="sticky-enabled table-select-processed tableheader-processed sticky-table"><thead><tr><th>uri</th><th>human-readable name</th> </tr></thead>';	
			// table += '<tbody><tr class="odd"><td colspan="2" class="empty message">' + xml + '</td> </tr>';
		// }		
		// table += '</tbody>';
		// table += '</table>';
		// $('#edit-tableselect-container').append(table);
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

function getResponse(input){
	console.log("Response for GET request: " + input);
	var regex = /<uri>(.*)<\/uri><method>(.*)<\/method><response>(.*)<\/response>/;
	var matches = regex.exec(input);
	if(matches){
		var label = '#lbl_' + matches[2] + "_" + matches[1];
		label = label.replace(/\:/g, '\\\:');
		label = label.replace(/\//g, '\\\/');
		$(label).hide();
		$(label).html(matches[3]);
		$(label).fadeIn('slow');
		resource_div = '#' + matches[1];
		resource_div = resource_div.replace(/\:/g, '\\\:');
		resource_div = resource_div.replace(/\//g, '\\\/');
		var img_status = $(resource_div + " > ." + matches[2] + " > .img_status");
		img_status.attr("src", Drupal.settings.basePath + Drupal.settings.coap_resource.module_path + "/images/valid.ico");
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
	console.log("Response for observe: " + input);
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