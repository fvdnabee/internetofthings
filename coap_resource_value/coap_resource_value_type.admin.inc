<?php

/**
 * @file
 * coap_resource_value type editing UI.
 */

/**
 * UI controller.
 */
class coap_resource_valueTypeUIController extends EntityDefaultUIController {

  /**
   * Overrides hook_menu() defaults.
   */
  public function hook_menu() {
    $items = parent::hook_menu();
		$items[$this->path]['description'] = 'Manage coap_resource_value entity types, including adding
		and removing fields and the display of fields.';
    return $items;
  }
}

/**
 * Generates the coap_resource_value type editing form.
 */
function coap_resource_value_type_form($form, &$form_state, $coap_resource_value_type, $op = 'edit') {

  if ($op == 'clone') {
    $coap_resource_value_type->label .= ' (cloned)';
    $coap_resource_value_type->type = '';
  }

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $coap_resource_value_type->label,
    '#description' => t('The human-readable name of this coap_resource_value type.'),
    '#required' => TRUE,
    '#size' => 30,
  );
  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($coap_resource_value_type->type) ? $coap_resource_value_type->type : '',
    '#maxlength' => 32,
//    '#disabled' => $coap_resource_value_type->isLocked() && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'coap_resource_value_get_types',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this coap_resource_value type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['data']['#tree'] = TRUE;
  $form['data']['sample_data'] = array(
    '#type' => 'checkbox',
    '#title' => t('An interesting coap_resource_value switch'),
    '#default_value' => !empty($coap_resource_value_type->data['sample_data']),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save coap_resource_value type'),
    '#weight' => 40,
  );

  //Locking not supported yet
  /*if (!$coap_resource_value_type->isLocked() && $op != 'add') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete coap_resource_value type'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('coap_resource_value_type_form_submit_delete')
    );
  }*/
  return $form;
}

/**
 * Form API submit callback for the type form.
 */
function coap_resource_value_type_form_submit(&$form, &$form_state) {
  $coap_resource_value_type = entity_ui_form_submit_build_entity($form, $form_state);
  $coap_resource_value_type->save();
  $form_state['redirect'] = 'admin/structure/coap_resource_value_types';
}

/**
 * Form API submit callback for the delete button.
 */
function coap_resource_value_type_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/coap_resource_value_types/manage/' . $form_state['coap_resource_value_type']->type . '/delete';
}
