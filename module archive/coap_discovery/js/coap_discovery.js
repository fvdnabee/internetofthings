(function ($) {

var refreshId;

Drupal.behaviors.coap_resource = {
  attach: 	function(context) {
				refreshId = setInterval(function(){
					$.ajax({
						type: "GET",
						url: "/coap_discovery/poll",
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
		var table = '';
		var links = xml.match(/<link>(.+?)<\/link>/g);
		if(links){
			table += '<table class="sticky-enabled table-select-processed tableheader-processed sticky-table"><thead><tr><th class="select-all"><input type="checkbox" class="form-checkbox" title="Select all rows in this table"></th><th>link name</th><th>human-readable name</th> </tr></thead>';	
			table += '<tbody>';
			for(var i=0 ; i< links.length ; i++){
				var link_attr = /<link><link_name>(.*?)<\/link_name><title>(.*?)<\/title><\/link>/.exec(links[i]);
				if(i%2 == 0){
					table += '<tr class="even">';
				}
				else{
					table += '<tr class="odd">';
				}
				table += '<td><div class="form-item form-type-checkbox form-item-table-' + i + '">' +
						 '<input type="checkbox" id="edit-table-' + i + '" name="table[' + i + ']" value="' + i + '" class="form-checkbox">' + 
						 '</div></td><td>' + link_attr[1] + '</td><td>' + link_attr[2] + '</td> </tr>';
			}
		}
		else{
			table += '<table class="sticky-enabled table-select-processed tableheader-processed sticky-table"><thead><tr><th>link name</th><th>human-readable name</th> </tr></thead>';	
			table += '<tbody><tr class="odd"><td colspan="2" class="empty message">' + xml + '</td> </tr>';
		}		
		table += '</tbody>';
		table += '</table>';
		$('#edit-tableselect-container').append(table);
	}
}
})(jQuery);