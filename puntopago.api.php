<?php

/**
 * @file
 * Implements functionality for to use Punto pago as payment gateway.
 *
 * This is a base module that has submodules to implement specific functions for
 * Ubercart and Drupal Coomerse. In case that a new commerce module appears, a
 * new Punto pago gateway's module can be created with this implementations.
 */

/**
 * Declares the hook_puntopago_commerce_system.
 * 
 * @return array
 *   An array with the following elements:
 *   - title: System's name
 *   - description: System's description
 * 	 - success: callback for success page
 * 	 - failure: callback for failiure page
 * 	 - close validate: callback to validate the order on closing page
 * 	 - order load: callback executed when validation on closing page, that
 *  loads the order information
 * 	 - save transaction(optional): callback invoked when the transaction is been
 *  saved on closing page
 *   - accept transaction(optional): callback invoked after all previous
 *  validations
 */
function hook_puntopago_puntopago_commerce_system() {
  $info = array();
  $info['my_commerce'] = array(
    'title' => 'My Commerce',
    'success' => 'my_commerce_puntopago_success',
    'failure' => 'my_commerce_puntopago_failure',
    'close validate' => 'my_commerce_puntopago_validate_close',
    'order load' => 'my_commerce_puntopago_order_load',
    'save transaction' => 'my_commerce_puntopago_save_transaction',
    'accept transaction' => 'my_commerce_puntopago_accept_transaction',
  );
  return $info;
}
