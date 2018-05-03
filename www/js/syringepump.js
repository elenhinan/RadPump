$(function(){

    // modal activity dropdown
    $("#input_isotope").change( function() {
        if (this.value == "None") {
            $("#collapse_activity").collapse('hide');
        } else {
            $("#collapse_activity").collapse('show');
        }
    });

    // setup datetimepicker
    $("#input_t0").datetimepicker({
        icons: {
            time: "fa fa-clock",
            date: "fa fa-calendar-alt",
            up: "fa fa-arrow-up",
            down: "fa fa-arrow-down"
        },
        showTodayButton: true,
        showClose: true,
        useCurrent: true,
        format: "YYYY/MM/DD HH:mm:ss",
        allowInputToggle: true
    });

    // event for insert modal apply button
    $("#button_insert_apply").click(function() {
        $("#modal_insert").modal('hide');
        send_insert();
    });

    // event for config save button
    $("#button_config_ok").click(function() {
        save_config();
    });
    $("#button_config_cancel").click(function() {
        reset_config();
    });

    // button bar events
    $("#button_home").click(function() { send_cmd( {
        'command':'home'
    });});
    //$("#button_insert").click, function() {
    $("#button_group_move").keydown(function(event) {
        event.preventDefault();
        console.info(event);
    });
    $("#button_out_nudge").click(send_nudge_out);
    $("#button_in_nudge").click(send_nudge_in);
    $("#button_out_move").mousedown(send_travel_out);
    $("#button_out_move").mouseup(send_travel_stop);
    $("#button_out_move").mouseleave(send_travel_stop);
    $("#button_in_move").mousedown(send_travel_in);
    $("#button_in_move").mouseup(send_travel_stop);
    $("#button_in_move").mouseleave(send_travel_stop);
    $("#button_start").click(send_start);

    // delay dropdown events
    $("#delay_unit_s").click(function(event) {
        $("#delay_unit").html("s");
        $('#delay_input').clone().attr('type','number').insertAfter('#delay_input').prev().remove();
        $('#delay_input').attr("style","padding-right:12px;text-align:right;");
    })
    $("#delay_unit_min").click(function(event) {
        $("#delay_unit").html("min");
        $('#delay_input').clone().attr('type','number').insertAfter('#delay_input').prev().remove();
        $('#delay_input').attr("style","padding-right:12px;text-align:right;");
    })
    $("#delay_unit_datetime").click(function(event) {
        $("#delay_unit").html("<span class=\"fas fa-calendar-alt\"></span>");
        $('#delay_input').clone().attr('type','text').insertAfter('#delay_input').prev().remove();
        $('#delay_input').attr("style","padding-right:27px;text-align:right;");
    })
    $('#delay_input').on('input', function(event) {
        var isValid=/^[0-9]+(\.[0-9]*)?$/.test($(this).val());
        if(!isValid)
            event.preventDefault();
    })
    
    // speed dropdown events
    $("#speed_unit_uls").click(function(event) {
        set_speed(speed.flow);
    })
    $("#speed_unit_s").click(function(event) {
        set_speed(speed.duration);
    })

    // amount dropdown events
    $("#amount_unit_volume").click(function(event) {
        set_unit(unit.volume);
    })
    $("#amount_unit_length").click(function(event) {
        set_unit(unit.length);
    })
    $("#amount_unit_activity").click(function(event) {
        set_unit(unit.activity);
    })
    $("#amount_unit_dosage").click(function(event) {
        set_unit(unit.dosage);
    })
});

// Date.prototype.toDateInputValue = (function() {
//     var local = new Date(this);
//     local.setMinutes(this.getMinutes() - this.getTimezoneOffset());
//     return local.toJSON().slice(0,10);
// });

$(document).ready(function() {
    // disable caching for ajax
    $.ajaxSetup({ cache: false });
    
    // read configuration values from cookies
    load_config();
    save_config();
    
    // fill modal insert box with syringes and isotopes available
    load_syringes();
    load_isotopes();

    // update fields with live data at regular interval
    update();
    var interval = setInterval(
        update,
        10000
    );

    // enable tooltips
    $('[data-tooltip="tooltip"]').tooltip({trigger:'hover'});
    
});

function send_cmd(param) {
    $.ajax({
        dataType: "json",
        url: "api",
        //method: "post",
        data: param,
        success: function() {
        }
    });
}

function send_datetime() {
    send_cmd( {
        'command':  'set_time',
        'datetime': Date.now().toString()
    });
}

function send_nudge_out() {
    send_cmd( {
        'command':  'move_rel',
        'x':        config.nudge_dist,
        'a':        config.nudge_accel,
        's':        config.nudge_speed
    });
}

function send_nudge_in() {
    send_cmd( {
        'command':  'move_rel',
        'x':       -config.nudge_dist,
        'a':        config.nudge_accel,
        's':        config.nudge_speed
    });
}

function send_travel_out() {
    travel = true;
    send_cmd( {
        'command':  'move_abs',
        'x':        position_max,
        'a':        config.travel_accel,
        's':        config.travel_speed
    });
}

function send_travel_in() {
    travel = true;
    send_cmd( {
        'command':  'move_abs',
        'x':        position_min,
        'a':        config.travel_accel,
        's':        config.travel_speed
    });
}

function send_travel_stop() {
    if(travel) {
        travel = false;
        send_cmd( {
            'command':  'stop'
        });
    }
}

function send_stop() {
    send_cmd( {
        'command':  'stop'
    });
}

function send_start() {
    send_cmd( {
        'command':  'start'
    });
}

function send_insert() {
    var config_data = {}
    config_data.command = 'insert';
    config_data.syringe = syringes_json[$("#input_syringe").val()];
    if($("#input_isotope").val() != "None") {
        config_data.isotope = isotopes_json[$("#input_isotope").val()];
        config_data.a0 = parseFloat($("#input_a0").val());
        config_data.t0 = $("#input_t0").val();
    }
    

    console.info(config_data);
    $.ajax({
        dataType: "json",
        url: "api",
        data: config_data,
        success: function() {
        }
    });
}

function load_syringes() {
    $.ajax({
        dataType: "json",
        url: "syringes.json",
        data: "",
        success: function(data) {
            syringes_json = data;
            syringes_json.forEach(function(syringe,index) {
                $("<option>").val(index).text(syringe.name).appendTo("#input_syringe");
            })
        }
    });
}

function load_isotopes() {
    $.ajax({
        dataType: "json",
        url: "isotopes.json",
        data: "",
        success: function(data) {
            isotopes_json = data;
            isotopes_json.forEach(function(isotope,index) {
                $("<option>").val(index).text(isotope.name).appendTo("#input_isotope");
            })
        }
    });
}

// update parameters from json
function update() {
    $.ajax({
        dataType: "json",
        url: "status.json",
        data: "",
        success: function(data) {
            var injector = data.injector[0]
            //$("#debugtext").text(JSON.stringify(injector));
            $("#amount_input").val(injector.plan.amount);
            $("#speed_input").val(injector.plan.duration);
            $("#delay_input").val(injector.plan.delay);
            
            // update progress bar
            if (injector.state == "running") {
                // animate progressbar
                $("#volume_progress_current").addClass("progress-bar-animated");
            } else {
                //disable progressbar animation
                $("#volume_progress_current").removeClass("progress-bar-animated")
            }
            position_min = injector.syringe.end;
            position_max = injector.syringe.start;
            position_target = injector.status.target;
            position_current = injector.status.position;
            var progress_target = ((position_target-position_min)/(position_max-position_min)*100).toFixed(1);
            var progress_current = ((position_current-position_min)/(position_max-position_min)*100).toFixed(1) - progress_target;
            $("#volume_progress_target").css('width', progress_target+'%').attr('aria-valuenow', progress_target);
            $("#volume_progress_current").css('width', progress_current+'%').attr('aria-valuenow', progress_current);
        }
    });
}

var config;
var syringes_json;
var isotopes_json;

var position_min;
var position_max;
var position_target;
var position_current;

var travel = false;

var mode = {
    idle: 0,
    primed: 1,
    moving: 2,
    fail: 3
}

var speed = {
    flow: 0,
    duration: 1
}

var unit = {
    volume: 0,
    activity: 1,
    length: 2,
    dosage: 3
}

function load_config() {
    config = Cookies.getJSON('config');
    if (config === undefined) {
        config = {
            'nudge_dist'    : 0.5,
            'nudge_accel'   : 20.0,
            'nudge_speed'   : 5.0,
            'travel_accel'  : 5.0,
            'travel_speed'  : 5.0
        }
    }
    var width_label = 120;
    var width_unit = 70;
    var data;
    $("#modal_config_body").append("<label>Nudge:</label>")
    data = {
        "label_width"   : width_label,
        "unit_width"    : width_unit,
        "label"         : "Distance",
        "id"            : "nudge_dist",
        "value"         : config.nudge_dist,
        "step"          : 0.1,
        "min"           : 0.1,
        "max"           : 3,
        "unit"          : "mm",
    }
    $("#modal_config_body").append($.templates("#template_input_number").render(data));
    data = {
        "label_width"   : width_label,
        "unit_width"    : width_unit,
        "label"         : "Speed",
        "id"            : "nudge_speed",
        "value"         : config.nudge_speed,
        "step"          : 0.1,
        "min"           : 0.1,
        "max"           : 10,
        "unit"          : "mm/s"
    }
    $("#modal_config_body").append($.templates("#template_input_number").render(data));
    data = {
        "label_width"   : width_label,
        "unit_width"    : width_unit,
        "label"         : "Acceleration",
        "id"            : "nudge_accel",
        "value"         : config.nudge_accel,
        "step"          : 0.1,
        "min"           : 1.0,
        "max"           : 20,
        "unit"          : "mm/s<sup>2</sup>"
    }
    $("#modal_config_body").append($.templates("#template_input_number").render(data));
    data = {
        "label_width"   : width_label,
        "unit_width"    : width_unit,
        "label"         : "Speed",
        "id"            : "travel_speed",
        "value"         : config.travel_speed,
        "step"          : 0.1,
        "min"           : 1.0,
        "max"           : 10,
        "unit"          : "mm/s"
    }
    $("#modal_config_body").append("<label>Travel:</label>")
    $("#modal_config_body").append($.templates("#template_input_number").render(data));
    data = {
        "label_width"   : width_label,
        "unit_width"    : width_unit,
        "label"         : "Acceleration",
        "id"            : "travel_accel",
        "value"         : config.travel_accel,
        "step"          : 0.1,
        "min"           : 1.0,
        "max"           : 20,
        "unit"          : "mm/s<sup>2</sup>"
    }
    $("#modal_config_body").append($.templates("#template_input_number").render(data));
}

function save_config() {
    config.nudge_dist = $("#input_nudge_dist").val();
    config.nudge_speed = $("#input_nudge_speed").val();
    config.nudge_accel = $("#input_nudge_accel").val();
    config.travel_speed = $("#input_travel_speed").val();
    config.travel_accel = $("#input_travel_accel").val();
    Cookies.set('config', config, { expires: 365, path: '/' });
}

function reset_config() {
    $("#input_nudge_dist").val(config.nudge_dist);
    $("#input_nudge_speed").val(config.nudge_speed);
    $("#input_nudge_accel").val(config.nudge_accel);
    $("#input_travel_speed").val(config.travel_speed);
    $("#input_travel_accel").val(config.travel_accel);
}

function set_speed(val) {
    switch(val) {
        case speed.flow:
            $("#speed_unit").html("µl/s");
            break;
        case speed.duration:
            $("#speed_unit").html("s");
            break;
    }
}

function set_unit(val) {
    switch(val) {
        case unit.volume:
            $("#amount_unit").html("µl");
            $("#group_weight").collapse('hide');
            break;
        case unit.activity:
            $("#amount_unit").html("MBq");
            $("#group_weight").collapse('hide');
            break;
        case unit.dosage:
            $("#amount_unit").html("MBq/g");
            $("#group_weight").collapse('show');
            break;
        case unit.length:
            $("#amount_unit").html("mm");
            $("#group_weight").collapse('hide');
            break;
    }
}