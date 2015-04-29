<?php

/**
 * @file
 * This file contains the primary pages of this module.
 *
 * The primary pages are the failure page and the success page.
 */


/**
 * Processes and HTML response on a failure.
 *
 * @return string
 *   HTML output of the page
 */
function puntopago_failure() {
  $exists_order = isset($_POST['TBK_ORDEN_COMPRA']);
  $exists_session = isset($_POST['TBK_ID_SESION']);
  if (!$exists_order || !$exists_session) {
    drupal_goto(variable_get('puntopago_redirect_no_order_failure', ''));
  }

  $session = puntopago_prepare_session();
  $order_id = $_POST['TBK_ORDEN_COMPRA'];

  $commerce_infos = module_invoke_all('puntopago_commerce_system');
  $function = $commerce_infos[$session['commerce']]['failure'];

  $transaction = puntopago_get_transaction_by_order($order_id, $session['commerce']);

  if (function_exists($function)) {
    $output = $function($order_id, $transaction);
  }
  else {
    $args = array('!commerce' => $session['commerce']);
    $message = t('<em>Failure</em> function is not defined by !commerce', $args);
    drupal_set_message(check_plain($message), 'error');
    $output = '...';
  }

  return $output;
}


/**
 * Processes and HTML response on a success case.
 *
 * @return string
 *   HTML output of the page
 */
function puntopago_success() {
  $exists_order   = isset($_POST['TBK_ORDEN_COMPRA']);
  $exists_session = isset($_POST['TBK_ID_SESION']);

  if (!$exists_order || !$exists_session) {
    drupal_goto(variable_get('puntopago_redirect_no_order_success', ''));
  }

  $session = puntopago_prepare_session();
  $order_id = $_POST['TBK_ORDEN_COMPRA'];
  $transaction = puntopago_get_transaction_by_order($order_id, $session['commerce']);
  $commerce_infos = module_invoke_all('puntopago_commerce_system');
  $function = $commerce_infos[$session['commerce']]['success'];

  if (function_exists($function)) {
    $output = $function($order_id, $transaction);
    // If the system detects an irregularity can return FALSE.
    // Then it is a failed transaction.
    if ($output === FALSE) {
      drupal_set_title(t('Order failed'));

      return puntopago_failure();
    }
  }
  else {
    $args = array('!commerce' => $session['commerce']);
    $message = t('<em>Success</em> function is not defined by !commerce.', $args);
    drupal_set_message(check_plain($message), 'error');
    $output = '...';
  }

  return $output;
}


/**
 * Processes and transactions for the closing page.
 */
function puntopago_close() {
  $puntopago_commerce_system = module_invoke_all('puntopago_commerce_system');

  /* Registers on Drupal's dlog , the parameters from $_SERVER variable */
  if (variable_get('puntopago_debug_puntopago_close', FALSE)) {
    puntopago_debug_watchdog_server_variable();
  }

  $currency = arg(2);

  if (!$currency || !puntopago_validate_currency($currency)) {
    echo PUNTOPAGO_REJECTED;
    drupal_exit();
  }

  if (!isset($_POST['TBK_ORDEN_COMPRA']) ||
    !isset($_POST['TBK_ID_TRANSACCION']) ||
    !isset($_POST['TBK_MAC'])) {
    echo PUNTOPAGO_REJECTED;
    drupal_exit();
  }

  $root     = puntopago_document_root();
  $session  = puntopago_prepare_session();
  $commerce = $session['commerce'];
  $uid = $session['id'];
  $transaction = puntopago_prepare_transaction();
  $callbacks = $puntopago_commerce_system[$commerce];

  if (!puntopago_save_transaction($transaction, $callbacks, $uid)) {
    echo PUNTOPAGO_REJECTED;
    drupal_exit();
  }

  $message = array();

  if ($_POST['TBK_RESPUESTA'] == 0) {

    $puntopago_settings = puntopago_get_settings_kcc();
    $path_log = $root . '/' . $puntopago_settings[$currency]['puntopago_ROOT_CGI'] . '/log';
    $path_check = $root . '/' . $puntopago_settings[$currency]['puntopago_ROOT_CGI'] . '/tbk_check_mac.';
    $temporal = $path_log . "/temporal.txt";
    $filename = $path_log . "/log_" . $_POST['TBK_ID_TRANSACCION'] . ".txt";

    puntopago_save_authorization_code($temporal);
    puntopago_save_post_variables($filename);
    $result = puntopago_invoke_tbk_check_mac($filename, $commerce, $path_check, $message);

    if (isset($result[0]) && $result[0] == PUNTOPAGO_CORRECT) {

      $order = puntopago_order_load($_POST['TBK_ORDEN_COMPRA'], $callbacks);
      $validation_order = puntopago_check_order($order, $callbacks, $message);

      if ($validation_order == PUNTOPAGO_ACCEPTED) {
        $validation_amount = puntopago_check_amount($order, $currency, $message);

        if ($validation_amount == PUNTOPAGO_ACCEPTED) {
          /*
          After all validation, the order is in the state pending. Just in case
          that the process is aborted by the user
          */
          puntopago_accept_transaction($order, $callbacks);
          echo PUNTOPAGO_ACCEPTED;
        }
        else {
          echo PUNTOPAGO_REJECTED;/* amount not validated */
        }
      }
      else {
        echo PUNTOPAGO_REJECTED;/* order not validated */
      }
    }
    else {
      echo PUNTOPAGO_REJECTED;/* MAC not validated */
    }
  }
  else {
    $message[] = t('Transaction not allowed');
    /*
     * Rejected by Transbank but still OK for commerce.
     */
    echo PUNTOPAGO_ACCEPTED;
  }
  puntopago_save_log($message);

  drupal_exit();
}


/**
 * Helper function to have an array with the transaction information.
 *
 * @return array
 *   An array with Punto pago parameters ready to be persisted.
 */
function puntopago_prepare_transaction() {
  return array(
    'tbk_transaccion'       => $_POST['TBK_TIPO_TRANSACCION'],
    'tbk_respuesta'         => $_POST['TBK_RESPUESTA'],
    'tbk_orden_compra'      => $_POST['TBK_ORDEN_COMPRA'],
    'tbk_id_session'        => $_POST['TBK_ID_SESION'],
    'tbk_cod_autorizacion'  => $_POST['TBK_CODIGO_AUTORIZACION'],
    'tbk_monto'             => $_POST['TBK_MONTO'],
    'tbk_nro_final_tarjeta' => $_POST['TBK_FINAL_NUMERO_TARJETA'],
    'tbk_fecha_expiracion'  => isset($_POST['TBK_FECHA_EXPIRACION']) ? $_POST['TBK_FECHA_EXPIRACION'] : '',
    'tbk_fecha_contable'    => $_POST['TBK_FECHA_CONTABLE'],
    'tbk_fecha_transaccion' => $_POST['TBK_FECHA_TRANSACCION'],
    'tbk_hora_transaccion'  => $_POST['TBK_HORA_TRANSACCION'],
    'tbk_id_transaccion'    => $_POST['TBK_ID_TRANSACCION'],
    'tbk_tipo_pago'         => $_POST['TBK_TIPO_PAGO'],
    'tbk_nro_cuotas'        => $_POST['TBK_NUMERO_CUOTAS'],
    'tbk_mac'               => $_POST['TBK_MAC'],
    'tbk_tasa_interes_max'  => isset($_POST['TBK_TASA_INTERES_MAX']) ? $_POST['TBK_TASA_INTERES_MAX'] : '1',
    'tbk_vci'               => isset($_POST['TBK_VCI']) ? $_POST['TBK_VCI'] : '',
    'currency'              => arg(2),
    'timestamp_transaction' => time(),
  );
}


/**
 * Saves the authorization code into a log file.
 *
 * @param string $temporal
 *   absolute path to the temporal log text file.
 */
function puntopago_save_authorization_code($temporal) {
  if ($fp = fopen($temporal, "w")) {
    fwrite($fp, $_POST['TBK_CODIGO_AUTORIZACION']);
    fclose($fp);
  }
}


/**
 * Saves $_POST variables into a log file.
 *
 * @param string $filename
 *   absolute path to the log file
 */
function puntopago_save_post_variables($filename) {
  $fp = fopen($filename, "w");
  reset($_POST);
  while (list($key, $val) = each($_POST)) {
    fwrite($fp, "$key=$val&");
  }
  fclose($fp);
}


/**
 * Validates MAC through a KCC script.
 *
 * @param string $filename
 *   file path and name for the KCC script that validates the MAC.
 * @param string $commerce
 *   Current used commerce's name.
 * @param string $path_check
 *   The tbk_check_mac file path.
 * @param array $message
 *   an array with all the steps followed in the close page, as strings.
 *
 * @return array
 *   command result.
 */
function puntopago_invoke_tbk_check_mac($filename, $commerce, $path_check, &$message) {
  $root = puntopago_document_root();
  $extension_path = variable_get('puntopago_tbk_check_mac_extension', 'cgi');
  $cmdline = $path_check . $extension_path . ' ' . $filename;
  $message[] = t('Commerce used: !commerce', array('!commerce' => $commerce));
  $message[] = t('Command: !cmd', array('!cmd' => $cmdline));

  /* MAC validation through the script*/
  exec($cmdline, $result, $retint);
  $cmd_result = (!is_array($result) || !isset($result[0]));
  if (!$cmd_result) {
    $message[] = t('Command result: !result', array('!result' => $result[0]));
  }
  else {
    $message[] = t('Command failed: !cmd', array('!cmd' => $cmdline));
  }

  return $result;
}


/**
 * Validates the order by its status.
 *
 * @param array $order
 *   Generic Punto pago's order.
 * @param array $message
 *   an array with all the steps followed in the close page, as strings.
 *
 * @return string
 *   - PUNTOPAGO_ACCEPTED if it is validated.
 *   - PUNTOPAGO_REJECTED if it is not validated.
 */
function puntopago_check_order($order, $commerce, &$message) {
  $validation_order = PUNTOPAGO_REJECTED;
  if (puntopago_validate_order_status($order, $commerce)) {
    $validation_order = PUNTOPAGO_ACCEPTED;
  }
  $args = array(
    '!validate_order' => $validation_order,
  );
  $message[] = t('Validates the order: !validate_order', $args);

  return $validation_order;
}


/**
 * Validates that the amount on Punto pago's order and commerce order are the same.
 *
 * @param array $order
 *   Generic Punto pago's order.
 * @param string $currency
 *   Currency code
 * @param array $message
 *   an array with all the steps followed in the close page, as strings.
 *
 * @return string
 *   - PUNTOPAGO_ACCEPTED if it is validated
 *   - PUNTOPAGO_REJECTED if it is not validated
 */
function puntopago_check_amount($order, $currency, &$message) {
  $amount = puntopago_amount($order['order_total'], $currency);
  $args = array(
    '!TBK_MONTO' => $_POST['TBK_MONTO'],
    '!order_total' => $amount,
  );
  $message[] = t('Compares TBK_MONTO: !TBK_MONTO with order_total: !order_total', $args);
  $validation_amount = PUNTOPAGO_REJECTED;
  if ($_POST['TBK_MONTO'] == $amount) {
    $validation_amount = PUNTOPAGO_ACCEPTED;
  }
  $args_2 = array('!validate' => $validation_amount);
  $message[] = t('Validates the amount: !validate', $args_2);

  return $validation_amount;
}


/**
 * Helper function to parse Punto pago information about session.
 *
 * @return array
 *   The array has the following keys.
 *     - commerce
 *     - id
 */
function puntopago_prepare_session() {
  $explode = explode(':', $_POST['TBK_ID_SESION']);
  return array(
    'commerce' => $explode[0],
    'id' => $explode[1],
  );
}


/**
 * Declares a pseudo hook that loads the order.
 *
 * @param int $order_id
 *   order id.
 * @param array $callbacks
 *   It is an array with commerce system's callbacks.
 *
 * @return array
 *   an array with the order and the other parameters the commerce may give.
 */
function puntopago_order_load($order_id, $callbacks) {
  $function = $callbacks['order load'];
  $order = array(
    'order_id' => $order_id,
  );
  if (function_exists($function)) {
    $function($order);
  }
  else {
    return FALSE;
  }
  return $order;
}


/**
 * Implements a pseudo hook.
 *
 * This validates the order status. This function must be declared depending
 * on the specific commerce system.
 *
 * @param array $order
 *   Order created with puntopago_order_load.
 * @param array $callbacks
 *   It is an array with commerce system's callbacks.
 *
 * @return bool
 *   true if the order exists and the function has beed declared, false
 *   otherwise.
 */
function puntopago_validate_order_status($order, $callbacks) {
  $function = $callbacks['close validate'];
  if (!$order) {
    return FALSE;
  }
  if (function_exists($function)) {
    return $function($order);
  }
  return FALSE;
}


/**
 * Function to invoke after all validations are ok.
 *
 * @param array $order
 *   Order created with puntopago_order_load.
 * @param array $callbacks
 *   It is an array with commerce system's callbacks.
 */
function puntopago_accept_transaction($order, $callbacks) {
  if (isset($callbacks['accept transaction'])) {
    $function = $callbacks['accept transaction'];
    if (function_exists($function)) {
      $function($order);
    }
  }
}


/**
 * Saves on Punto pago's log the transaction.
 *
 * Also invokes save_transaction function from the commerce submodule.
 *
 * @param array $transaction
 *   Corresponde a la transaccion enviada por puntopago.
 * @param array $callbacks
 *   It is an array with commerce system's callbacks.
 */
function puntopago_save_transaction($transaction, $callbacks, $uid) {

  $query = db_insert('puntopago')
    ->fields($transaction);

  try {
    $wid = $query->execute();
    $transaction['wid'] = $wid;
    if (isset($callbacks['save transaction'])) {
      $function = $callbacks['save transaction'];
      if (function_exists($function)) {
        $function($transaction, $uid);
      }
    }
    return TRUE;
  }
  catch (PDOException $err) {
    $watchdog_message = 'Failed to save the transaction in the database: @err';
    $watchdog_variable = array('@err' => $err->getMessage());
    watchdog('PUNTOPAGO', $watchdog_message, $watchdog_variable, WATCHDOG_ERROR);
    return FALSE;
  }
}
