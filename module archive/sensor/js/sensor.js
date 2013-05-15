(function ($) {

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
}
})(jQuery);