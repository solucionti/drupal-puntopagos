<?php

/**
 * @file
 * This file has the administrative pages for this module.
 */

/**
 * Helper debug function.
 *
 * @return string
 *   HTML string
 */
function puntopago_admin_debug() {
    $output = '<h2>' . t('Logs') . '</h2>';
    $output .= t('Here you can see the sequence of a transaction on close page');
    $output .= puntopago_get_log();
    return $output;
}

/**
 * Return a table with all logs related to Punto Pago's KCC.
 *
 * @return string
 *   HTML table with pager
 */
function puntopago_admin_log() {
    $transaction = puntopago_get_all_puntopago_transaction(TRUE);
    $output      = t('No log recorded');
    if (!empty($transaction)) {
        foreach ($transaction[0] as $key => $value) {
            $header[] = $key;
        }
        foreach ($transaction as $value) {
            $rows[] = $value;
        }
        $output = theme('table', array('rows' => $rows, 'header' => $header));
        $output .= theme('pager', array());
    }
    return $output;
}

/**
 * Get a form for all configurations.
 *
 * @param mixed $form
 *   form element
 * @param mixed $form_state
 *   current form state
 *
 * @return array
 *   form element
 */
function puntopago_configuration($form, $form_state) {
    $form += puntopago_commerce_system_config();
    $form += puntopago_basic_config();
    $form += puntopago_redirect_pages_config();
    return system_settings_form($form);
}

/**
 * Defines de basic_config form.
 *
 * @return array
 *   form element
 */
function puntopago_basic_config() {
    $form['puntopago_basic_config']                             = array(
        '#type'        => 'fieldset',
        '#title'       => t('Basic Settings'),
        '#collapsible' => TRUE,
        '#collapsed'   => TRUE,
    );
    $form['puntopago_basic_config']['puntopago_devolucion_url'] = array(
        '#type'          => 'textfield',
        '#title'         => t('Refund URL'),
        '#description'   => t('Example: "node/1" .Transbank requires a url that gives information about refunds.'),
        '#default_value' => variable_get('puntopago_devolucion_url', ''),
        '#size'          => 19,
        '#maxlength'     => 255,
    );

    $form['puntopago_basic_config']['puntopago_debug_puntopago_close'] = array(
        '#type'          => 'checkbox',
        '#title'         => t('Debug the puntopago close page'),
        '#description'   => t('Registers $_SERVER values on POST action to close page. Just for development purpose'),
        '#default_value' => variable_get('puntopago_debug_puntopago_close', FALSE),
    );

    $form['puntopago_basic_config']['puntopago_active_access'] = array(
        '#type'          => 'checkbox',
        '#title'         => t('Active Access Restriction'),
        '#description'   => t('Activates access restriction for IPs on dev or prod enviroment'),
        '#default_value' => variable_get('puntopago_active_access', FALSE),
    );

    return $form;
}

/**
 * Form to define de commerce system for the site.
 *
 * @return array
 *   form element
 */
function puntopago_commerce_system_config() {
    $form['puntopago_commerce_system'] = array(
        '#type'        => 'fieldset',
        '#title'       => t('Activated Electronic Commerce'),
        '#collapsible' => TRUE,
    );
    $infos                             = module_invoke_all('puntopago_commerce_system');
    $rows                              = array();
    if (!empty($infos)) {
        foreach ($infos as $key => $info) {
            $rows[] = array(
                $info['title'] . " ($key)",
                isset($info['description']) ? $info['description'] : '',
            );
        }
    }

    $form['puntopago_commerce_system']['systems'] = array(
        '#theme'  => 'table',
        '#rows'   => $rows,
        '#empty'  => t('You need Active a Commerce System'),
        '#header' => array(t('System'), t('Description')),
    );

    return $form;
}

/**
 * Form to retrieve redirections on different cases.
 *
 * @return array
 *   form element
 */
function puntopago_redirect_pages_config() {
    $form['redirect_pages_no_order'] = array(
        '#type'        => 'fieldset',
        '#title'       => t('Redirect pages if no order on POST'),
        '#collapsible' => TRUE,
        '#collapsed'   => TRUE,
    );

    $form['redirect_pages_no_order']['puntopago_redirect_no_order_failure'] = array(
        '#type'          => 'textfield',
        '#title'         => t('Redirect page from Failure'),
        '#default_value' => variable_get('puntopago_redirect_no_order_failure', ''),
    );

    $form['redirect_pages_no_order']['puntopago_redirect_no_order_success'] = array(
        '#type'          => 'textfield',
        '#title'         => t('Redirect page from Success'),
        '#default_value' => variable_get('puntopago_redirect_no_order_success', ''),
    );

    return $form;
}

/**
 * Form for Webapay's KCC's values.
 *
 * @param array $form
 *   form element
 * @param array $form_state
 *   current form state
 *
 * @return array
 *   form element
 */
function puntopago_configuration_kcc($form, $form_state) {
    $form['puntopago_kcc_general'] = array(
        '#type'        => 'fieldset',
        '#title'       => t('General Data'),
        '#description' => t('This parameters will be written on tbk_config.dat file. Apache must have write permission'),
        '#collapsible' => TRUE,
        '#collapsed'   => TRUE,
    );

    $form['puntopago_kcc_general']['puntopago_AMBIENTE'] = array(
        '#type'          => 'select',
        '#title'         => t('Ambiente de Punto Pago'),
        '#options'       => array(
            'https://sandbox.puntopagos.com' => 'Sandbox',
            'https://www.puntopagos.com'     => 'Producción',
        ),
        '#default_value' => variable_get('puntopago_AMBIENTE', 'cgi'),
    );

    $form['puntopago_kcc_general']['puntopago_KEY']    = array(
        '#type'          => 'textfield',
        '#title'         => t('KEY'),
        '#description'   => t('Fill with your punto pago KEY'),
        '#size'          => 60,
        '#maxlength'     => 90,
        '#default_value' => variable_get('puntopago_KEY', 'cgi')
    );
    $form['puntopago_kcc_general']['puntopago_SECRET'] = array(
        '#type'          => 'textfield',
        '#title'         => t('SECRET'),
        '#description'   => t('Fill with your punto pago SECRET'),
        '#size'          => 60,
        '#maxlength'     => 90,
        '#default_value' => variable_get('puntopago_SECRET', 'cgi')
    );


    $form['#submit'][] = 'puntopago_edit_files';

    return system_settings_form($form);
}

/**
 * Validation for puntopago_kcc_config().
 *
 * @param array $form
 *   form element
 * @param array $form_state
 *   current form state
 */
function puntopago_admin_kcc_config_validate($form, &$form_state) {
    $values    = $form_state['values'];
    $path_root = puntopago_document_root() . '/';

    $values['puntopago_ROOT_CGI_CLP'] = trim($values['puntopago_ROOT_CGI_CLP']);
    $values['puntopago_ROOT_CGI_USD'] = trim($values['puntopago_ROOT_CGI_USD']);

    $no_empty_clp  = ($values['puntopago_ROOT_CGI_CLP'] != '');
    $no_empty_usd  = ($values['puntopago_ROOT_CGI_USD'] != '');
    $equal_clp_usd = ($values['puntopago_ROOT_CGI_CLP'] == $values['puntopago_ROOT_CGI_USD']);

    if ($no_empty_clp && $no_empty_usd && $equal_clp_usd) {
        $msg = t('If you want both currencies then you will need to install 2 KCCs');
        form_set_error('puntopago_ROOT_CGI_USD', $msg);
        return;
    }

    $currs = array();
    if ($no_empty_clp) {
        $currs[] = 'CLP';
    } elseif ($no_empty_usd) {
        $currs[] = 'USD';
    }

    if ($no_empty_clp || $no_empty_usd) {
        foreach ($currs as $curr) {
            $tbk_config = $path_root . $values['puntopago_ROOT_CGI_' . $curr];
            $tbk_config .= '/datos/tbk_config.dat';

            if (!file_exists($tbk_config)) {
                $msg = t('Invalid path or tbk_config.dat not found');
                form_set_error('puntopago_ROOT_CGI_' . $curr, $msg);
            }
        }
    }
}

/**
 * Writes the file tbk_config.dat.
 *
 * @param array $form
 *   form element
 * @param array $form_state
 *   current form state
 */
function puntopago_edit_files($form, $form_state) {
    global $base_url;
    $environments = puntopago_get_environment();
    $settings     = $form_state['values'];

    foreach (array('CLP', 'USD') as $currency) {
        if ($settings['puntopago_ROOT_CGI_' . $currency] == '') {
            continue;
        }

        $filename = 'tbk_config.dat.' . $currency;
        $uri      = 'public://' . $filename;
        $file     = drupal_realpath($uri);
        $fp       = fopen($file, "w");

        $environment     = $environments[$settings['puntopago_environment_' . $currency]];
        $data_tbk_config = array(
            'IDCOMERCIO  = ' . $settings['puntopago_IDCOMERCIO_' . $currency],
            'MEDCOM  = ' . $settings['puntopago_MEDCOM'],
            'PARAMVERIFCOM = ' . $settings['puntopago_PARAMVERIFCOM_' . $currency],
            'TBK_KEY_ID  = ' . $settings['puntopago_TBK_KEY_ID_' . $currency],
            'URLCGICOM = ' . $base_url . '/' . $settings['puntopago_ROOT_CGI_' . $currency] . '/tbk_bp_resultado.cgi',
            'SERVERCOM = ' . $settings['puntopago_SERVERCOM'],
            'PORTCOM = ' . $settings['puntopago_PORTCOM'],
            'WHITELISTCOM = ' . $settings['puntopago_WHITELISTCOM'],
            'HOST = ' . $settings['puntopago_HOST'],
            'WPORT = ' . $settings['puntopago_WPORT'],
            'URLCGITRA = ' . $settings['puntopago_URLCGITRA'],
            'URLCGIMEDTRA = ' . $settings['puntopago_URLCGIMEDTRA'],
            'SERVERTRA = ' . $environment['servertra'],
            'PORTTRA = ' . $environment['porttra'],
            'PREFIJO_CONF_TR = ' . $settings['puntopago_PREFIJO_CONF_TR'],
            'HTML_TR_NORMAL = ' . $base_url . '/?q=puntopago/close/' . $currency,
        );
        foreach ($data_tbk_config as $data) {
            fwrite($fp, $data . "\n");
        }
        fclose($fp);

        $tbk_config = puntopago_document_root() . '/' . $settings['puntopago_ROOT_CGI_' . $currency] . '/datos/tbk_config.dat';

        if (!is_writable($tbk_config)) {
            $url = file_create_url($uri);

            $args = array(
                '@currency'   => $currency,
                '%filename'   => $filename,
                '@tbk_config' => $tbk_config,
                '@url'        => $url,
            );
            drupal_set_message(t('It seems Apache does not have permission to copy the file tbk_config.dat on KCC for the currency @currency. You can download the file from <a href="@url">here</a> and replace it for @tbk_config', $args), 'warning');
        } else {
            if (copy($file, $tbk_config)) {
                $args = array(
                    '@currency'   => $currency,
                    '@tbk_config' => $tbk_config,
                );
                drupal_set_message(t('tbk_config.dat for @currency has been created and has been copied in @tbk_config', $args));
            }
        }
    }
}
