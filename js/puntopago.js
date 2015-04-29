/**
 * @file
 * Redirects users to Punto Pago.
 * 
 * This plugin has been developed to be used on an AJAX call though the command
 * ajax_command_invoke.
 * Example: ajax_command_invoke(null,'puntopagoRedirect',array($params));
 * Regenerates a form with the paramaters and the submission is been made.
 *
 * To use this script it needs to be called with the function
 * puntopago_redirect_js($currency). Here the variable
 * Drupal.settings.puntopago.actionForm is defined.
 */

/* global Drupal, jQuery */
(function ($) {
    $.fn.puntopagoRedirect = function (params) {
        var form, param;
        $('input[type=submit]').attr('disabled', 'disabled');
        form = $("<form>", {
            action: Drupal.settings.puntopago.actionForm,
            method: "POST"
        });
        for (param in params) {
            if (param.indexOf("TBK_") !== -1) {
                form.append($('<input/>', {
                    type: "hidden",
                    name: param,
                    value: params[param]
                }));
            }
        }

        form.appendTo('body');
        form.submit();
    };

})(jQuery);
