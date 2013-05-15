<?php

/**
 * @file
 * Coap_resource_value editing UI.
 *
 * We make very little use of the EntityAPI interface for this - preferring instead to use
 * views. That offers more flexibility to change a UI that will, more often than not,
 * be end-user facing.
 */

/**
 * UI controller.
 */
class Coap_resource_valueUIController extends EntityDefaultUIController {

  /**
   * Overrides hook_menu() defaults. Main reason for doing this is that
   * parent class hook_menu() is optimized for entity type administration.
   */
  public function hook_menu() {
    
    $items = array();
    $id_count = count(explode('/', $this->path));
    $wildcard = isset($this->entityInfo['admin ui']['menu wildcard']) ? $this->entityInfo['admin ui']['menu wildcard'] : '%' . $this->entityType;

    $items[$this->path] = array(
      'title' => 'Coap_resource_values',
      'description' => 'Add edit and update coap_resource_values.',
      'page callback' => 'system_admin_menu_block_page',
      'access arguments' => array('access administration pages'),
      'file path' => drupal_get_path('module', 'system'),
      'file' => 'system.admin.inc',
    );
    
    // Change the overview menu type for the list of coap_resource_values.
    $items[$this->path]['type'] = MENU_LOCAL_TASK;
    
    // Change the add page menu to multiple types of entities
    $items[$this->path . '/add'] = array(
      'title' => 'Add a coap_resource_value',
      'description' => 'Add a new coap_resource_value',
      'page callback'  => 'coap_resource_value_add_page',
      'access callback'  => 'coap_resource_value_access',
      'access arguments' => array('edit'),
      'type' => MENU_NORMAL_ITEM,
      'weight' => 20,
      'file' => 'coap_resource_value.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module'])

    );
    
    // Add menu items to add each different type of entity.
    foreach (coap_resource_value_get_types() as $type) {
      $items[$this->path . '/add/' . $type->type] = array(
        'title' => 'Add ' . $type->label,
        'page callback' => 'coap_resource_value_form_wrapper',
        'page arguments' => array(coap_resource_value_create(array('type' => $type->type))),
        'access callback' => 'coap_resource_value_access',
        'access arguments' => array('edit', 'edit ' . $type->type),
        'file' => 'coap_resource_value.admin.inc',
        'file path' => drupal_get_path('module', $this->entityInfo['module'])
      );
    }

    // Loading and editing coap_resource_value entities
    $items[$this->path . '/coap_resource_value/' . $wildcard] = array(
      'page callback' => 'coap_resource_value_form_wrapper',
      'page arguments' => array($id_count + 1),
      'access callback' => 'coap_resource_value_access',
      'access arguments' => array('edit', $id_count + 1),
      'weight' => 0,
      'context' => MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE,
      'file' => 'coap_resource_value.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module'])
    );
    $items[$this->path . '/coap_resource_value/' . $wildcard . '/edit'] = array(
      'title' => 'Edit',
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'weight' => -10,
      'context' => MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE,
    );
    
    $items[$this->path . '/coap_resource_value/' . $wildcard . '/delete'] = array(
      'title' => 'Delete',
      'page callback' => 'coap_resource_value_delete_form_wrapper',
      'page arguments' => array($id_count + 1),
      'access callback' => 'coap_resource_value_access',
      'access arguments' => array('edit', $id_count + 1),
      'type' => MENU_LOCAL_TASK,
      'context' => MENU_CONTEXT_INLINE,
      'weight' => 10,
      'file' => 'coap_resource_value.admin.inc',
      'file path' => drupal_get_path('module', $this->entityInfo['module'])
    );
    
    // Menu item for viewing coap_resource_values
    $items['coap_resource_value/' . $wildcard] = array(
      //'title' => 'Title',
      'title callback' => 'coap_resource_value_page_title',
      'title arguments' => array(1),
      'page callback' => 'coap_resource_value_page_view',
      'page arguments' => array(1),
      'access callback' => 'coap_resource_value_access',
      'access arguments' => array('view', 1),
      'type' => MENU_CALLBACK,
    );
    return $items;
  }
  
  /**
   * Create the markup for the add Coap_resource_value Entities page within the class
   * so it can easily be extended/overriden.
   */ 
  public function addPage() {
    $item = menu_get_item();
    $content = system_admin_menu_block($item);

    if (count($content) == 1) {
      $item = array_shift($content);
      drupal_goto($item['href']);
    }    
        
    return theme('coap_resource_value_add_list', array('content' => $content));
  }
  
}

/**
 * Form callback wrapper: create or edit a coap_resource_value.
 *
 * @param $coap_resource_value
 *   The coap_resource_value object being edited by this form.
 *
 * @see coap_resource_value_edit_form()
 */
function coap_resource_value_form_wrapper($coap_resource_value) {
  // Add the breadcrumb for the form's location.
  coap_resource_value_set_breadcrumb();
  return drupal_get_form('coap_resource_value_edit_form', $coap_resource_value);
}

/**
 * Form callback wrapper: delete a coap_resource_value.
 *
 * @param $coap_resource_value
 *   The coap_resource_value object being edited by this form.
 *
 * @see coap_resource_value_edit_form()
 */
function coap_resource_value_delete_form_wrapper($coap_resource_value) {
  // Add the breadcrumb for the form's location.
  //coap_resource_value_set_breadcrumb();
  return drupal_get_form('coap_resource_value_delete_form', $coap_resource_value);
}

/**
 * Form callback: create or edit a coap_resource_value.
 *
 * @param $coap_resource_value
 *   The coap_resource_value object to edit or for a create form an empty coap_resource_value object
 *     with only a coap_resource_value type defined.
 */
function coap_resource_value_edit_form($form, &$form_state, $coap_resource_value) {
  // Add the default field elements.
  $form['payload'] = array(
    '#type' => 'textfield',
    '#title' => t('Coap_resource_value Payload'),
    '#default_value' => isset($coap_resource_value->payload) ? $coap_resource_value->payload : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#weight' => -5,
  );
  
  $form['uid'] = array(
    '#type' => 'textfield',
    '#title' => t('Coap_resource_value User ID'),
    '#default_value' => isset($coap_resource_value->uid) ? $coap_resource_value->uid : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#weight' => -4,
  );
  
  $form['content_format'] = array(
    '#type' => 'textfield',
    '#title' => t('Coap_resource_value Content Format'),
    '#default_value' => isset($coap_resource_value->content_format) ? $coap_resource_value->content_format : '',
    '#maxlength' => 255,
    '#required' => FALSE,
    '#weight' => -3,
  );
  
  $form['parsed_value'] = array(
    '#type' => 'textfield',
    '#title' => t('Coap_resource_value Parsed Value'),
    '#default_value' => isset($coap_resource_value->parsed_value) ? $coap_resource_value->parsed_value : '',
    '#maxlength' => 255,
    '#required' => FALSE,
    '#weight' => -1,
  );
  
  $form['max_age'] = array(
    '#type' => 'textfield',
    '#title' => t('Coap_resource_value Max Age'),
    '#default_value' => isset($coap_resource_value->max_age) ? $coap_resource_value->max_age : '',
    '#maxlength' => 255,
    '#required' => FALSE,
    '#weight' => 0,
  );
  
  $form['original_response'] = array(
    '#type' => 'textfield',
    '#title' => t('Coap_resource_value Original Response'),
    '#default_value' => isset($coap_resource_value->original_response) ? $coap_resource_value->original_response : '',
    '#maxlength' => 255,
    '#required' => FALSE,
    '#weight' => 1,
  );
  
  $form['data']['#tree'] = TRUE;
  $form['data']['sample_data'] = array(
    '#type' => 'checkbox',
    '#title' => t('An interesting coap_resource_value switch'),
    '#default_value' => isset($coap_resource_value->data['sample_data']) ? $coap_resource_value->data['sample_data'] : 1,
  );

  // Add the field related form elements.
  $form_state['coap_resource_value'] = $coap_resource_value;
  field_attach_form('coap_resource_value', $coap_resource_value, $form, $form_state);

  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('form-actions')),
    '#weight' => 400,
  );

  // We add the form's #submit array to this button along with the actual submit
  // handler to preserve any submit handlers added by a form callback_wrapper.
  $submit = array();

  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save coap_resource_value'),
    '#submit' => $submit + array('coap_resource_value_edit_form_submit'),
  );
  
  if (!empty($coap_resource_value->payload)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete coap_resource_value'),
      '#suffix' => l(t('Cancel'), 'admin/content/coap_resource_values'),
      '#submit' => $submit + array('coap_resource_value_form_submit_delete'),
      '#weight' => 45,
    );
  }

  // We append the validate handler to #validate in case a form callback_wrapper
  // is used to add validate handlers earlier.
  $form['#validate'][] = 'coap_resource_value_edit_form_validate';
  return $form;
}

/**
 * Form API validate callback for the coap_resource_value form
 */
function coap_resource_value_edit_form_validate(&$form, &$form_state) {
  $coap_resource_value = $form_state['coap_resource_value'];
  
  // Notify field widgets to validate their data.
  field_attach_form_validate('coap_resource_value', $coap_resource_value, $form, $form_state);
}

/**
 * Form API submit callback for the coap_resource_value form.
 * 
 * @todo remove hard-coded link
 */
function coap_resource_value_edit_form_submit(&$form, &$form_state) {
  
  $coap_resource_value = entity_ui_controller('coap_resource_value')->entityFormSubmitBuildEntity($form, $form_state);
  // Save the coap_resource_value and go back to the list of coap_resource_values
  
  // Add in created and changed times.
  if ($coap_resource_value->is_new = isset($coap_resource_value->is_new) ? $coap_resource_value->is_new : 0){
    $coap_resource_value->created = time();
  }

  $coap_resource_value->changed = time();
  
  $coap_resource_value->save();
  $form_state['redirect'] = 'admin/content/coap_resource_values';
}

/**
 * Form API submit callback for the delete button.
 * 
 * @todo Remove hard-coded path
 */
function coap_resource_value_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/content/coap_resource_values/coap_resource_value/' . $form_state['coap_resource_value']->coap_resource_value_id . '/delete';
}

/**
 * Form callback: confirmation form for deleting a coap_resource_value.
 *
 * @param $coap_resource_value
 *   The coap_resource_value to delete
 *
 * @see confirm_form()
 */
function coap_resource_value_delete_form($form, &$form_state, $coap_resource_value) {
  $form_state['coap_resource_value'] = $coap_resource_value;

  $form['#submit'][] = 'coap_resource_value_delete_form_submit';

  $form = confirm_form($form,
    t('Are you sure you want to delete coap_resource_value %payload?', array('%payload' => $coap_resource_value->payload)),
    'admin/content/coap_resource_values/coap_resource_value',
    '<p>' . t('This action cannot be undone.') . '</p>',
    t('Delete'),
    t('Cancel'),
    'confirm'
  );
  
  return $form;
}

/**
 * Submit callback for coap_resource_value_delete_form
 */
function coap_resource_value_delete_form_submit($form, &$form_state) {
  $coap_resource_value = $form_state['coap_resource_value'];

  coap_resource_value_delete($coap_resource_value);

  drupal_set_message(t('The coap_resource_value %payload has been deleted.', array('%payload' => $coap_resource_value->payload)));
  watchdog('coap_resource_value', 'Deleted coap_resource_value %payload.', array('%payload' => $coap_resource_value->payload));

  $form_state['redirect'] = 'admin/content/coap_resource_values';
}


/**
 * Page to add Coap_resource_value Entities.
 *
 * @todo Pass this through a proper theme function
 */
function coap_resource_value_add_page() {
  $controller = entity_ui_controller('coap_resource_value');
  return $controller->addPage();
}

/**
 * Displays the list of available coap_resource_value types for coap_resource_value creation.
 *
 * @ingroup themeable
 */
function theme_coap_resource_value_add_list($variables) {
  $content = $variables['content'];
  $output = '';
  if ($content) {
    $output = '<dl class="coap_resource_value-type-list">';
    foreach ($content as $item) {
      $output .= '<dt>' . l($item['title'], $item['href']) . '</dt>';
      $output .= '<dd>' . filter_xss_admin($item['description']) . '</dd>';
    }
    $output .= '</dl>';
  }
  else {
    if (user_access('administer coap_resource_value types')) {
      $output = '<p>' . t('Coap_resource_value Entities cannot be added because you have not created any coap_resource_value types yet. Go to the <a href="@create-coap_resource_value-type">coap_resource_value type creation page</a> to add a new coap_resource_value type.', array('@create-coap_resource_value-type' => url('admin/structure/coap_resource_value_types/add'))) . '</p>';
    }
    else {
      $output = '<p>' . t('No coap_resource_value types have been created yet for you to use.') . '</p>';
    }
  }

  return $output;
}

/**
 * Sets the breadcrumb for administrative coap_resource_value pages.
 */
function coap_resource_value_set_breadcrumb() {
  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Administration'), 'admin'),
    l(t('Content'), 'admin/content'),
    l(t('Coap_resource_values'), 'admin/content/coap_resource_values'),
  );

  drupal_set_breadcrumb($breadcrumb);
}



