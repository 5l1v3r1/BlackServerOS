
jQuery(document).ready(function() {
    
    if( $("#ssid").val().substr($("#ssid").length-4) == "off" ){
            $("#psk").val("key_mgmt_none");
            $("#psk").attr("readonly", true);
            $("#wifi_pass_div").html("<b>This WiFi network does not require a password, please proceed.</b>");
    }
	
    /*
        Fullscreen background
    */
    $.backstretch("/static/img/backgrounds/1.jpg");
    
    /*
        Form
    */
    
    // next step
    $('.registration-form .btn-next').on('click', function() {
    	var parent_fieldset = $(this).parents('fieldset');
    	var next_step = true;
    	
    	parent_fieldset.find('input[type="text"], input[type="password"], textarea').each(function() {
    		if( $(this).val() == "" ) {
    			$(this).addClass('input-error');
    			next_step = false;
    		}
    		else {
    			$(this).removeClass('input-error');
    		}
    	});
    	
    	if( next_step ) {
    		parent_fieldset.fadeOut(400, function() {
	    		$(this).next().fadeIn();
	    	});
    	}
    	
    });
    
    // previous step
    $('.registration-form .btn-previous').on('click', function() {
    	$(this).parents('fieldset').fadeOut(400, function() {
    		$(this).prev().fadeIn();
    	});
    });
    
    // submit
    $("#wifiandvpnform").on('submit', function(e) {
        var isValid = true;
    	$(this).find('input[type="text"], input[type="password"]').each(function() {
    		if( $(this).val() == "" ) {
    			e.preventDefault();
    			$(this).addClass('input-error');
                isValid = false;
    		}
    		else {
    			$(this).removeClass('input-error');
    		}
    	});
        if(isValid == true){
            $("#message-div").html('Please wait... you will be connected in less than 30 seconds.<br><i class="fa fa-spinner fa-pulse fa-3x fa-fw"></i><span class="sr-only">Loading...</span>');
        }
    });

    $("#ssid").change(function() {
        if( $(this).val().substr($(this).length-4) == "off" ){
            $("#psk").val("key_mgmt_none");
            $("#psk").attr("readonly", true);
            $("#wifi_pass_div").html("<b>This WiFi network does not require a password, please proceed.</b>");
        }
        else{
            $("#psk").attr("readonly", false);
            $("#psk").val("");
            $("#wifi_pass_div").html("");
        }
    });
    
});