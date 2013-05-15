<?php

/**
 * @file
 * Providing extra functionality for the Coap_resource_value UI via views.
 */


/**
 * Implements hook_views_data()
 */
function coap_resource_value_views_data_alter(&$data) { 
  $data['coap_resource_value']['link_coap_resource_value'] = array(
    'field' => array(
      'title' => t('Link'),
      'help' => t('Provide a link to the coap_resource_value.'),
      'handler' => 'coap_resource_value_handler_link_field',
    ),
  );
  $data['coap_resource_value']['edit_coap_resource_value'] = array(
    'field' => array(
      'title' => t('Edit Link'),
      'help' => t('Provide a link to the edit form for the coap_resource_value.'),
      'handler' => 'coap_resource_value_handler_edit_link_field',
    ),
  );
  $data['coap_resource_value']['delete_coap_resource_value'] = array(
    'field' => array(
      'title' => t('Delete Link'),
      'help' => t('Provide a link to delete the coap_resource_value.'),
      'handler' => 'coap_resource_value_handler_delete_link_field',
    ),
  );
  // This content of this field are decided based on the menu structure that
  // follows coap_resource_values/coap_resource_value/%coap_resource_value_id/op
  $data['coap_resource_value']['operations'] = array(
    'field' => array(
      'title' => t('Operations links'),
      'help' => t('Display all operations available for this coap_resource_value.'),
      'handler' => 'coap_resource_value_handler_coap_resource_value_operations_field',
    ),
  );
}


/**
 * Implements hook_views_default_views().
 */
function coap_resource_value_views_default_views() {
  $views = array();

  $view = new view;
  $view->name = 'coap_resource_values';
  $view->description = 'A list of all coap_resource_values';
  $view->tag = 'coap_resource_values';
  $view->base_table = 'coap_resource_value';
  $view->human_name = 'Coap_resource_values';
  $view->core = 7;
  $view->api_version = '3.0-alpha1';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  
  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Coap_resource_values';
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['access']['perm'] = 'create any coap_resource_value type';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '10';
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'payload' => 'payload',
    'coap_resource_value_id' => 'coap_resource_value_id',
  );
  $handler->display->display_options['style_options']['default'] = '-1';
  $handler->display->display_options['style_options']['info'] = array(
    'payload' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
    ),
    'coap_resource_value_id' => array(
      'sortable' => 0,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
    ),
  );
  $handler->display->display_options['style_options']['override'] = 1;
  $handler->display->display_options['style_options']['sticky'] = 0;
  $handler->display->display_options['style_options']['empty_table'] = 0;
  /* No results behavior: Global: Text area */
  $handler->display->display_options['empty']['area']['id'] = 'area';
  $handler->display->display_options['empty']['area']['table'] = 'views';
  $handler->display->display_options['empty']['area']['field'] = 'area';
  $handler->display->display_options['empty']['area']['label'] = 'Empty ';
  $handler->display->display_options['empty']['area']['empty'] = FALSE;
  $handler->display->display_options['empty']['area']['content'] = 'No coap_resource_values have been created yet';
  /* Field: Coap_resource_value: Coap_resource_value ID */
  $handler->display->display_options['fields']['coap_resource_value_id']['id'] = 'coap_resource_value_id';
  $handler->display->display_options['fields']['coap_resource_value_id']['table'] = 'coap_resource_value';
  $handler->display->display_options['fields']['coap_resource_value_id']['field'] = 'coap_resource_value_id';
  $handler->display->display_options['fields']['coap_resource_value_id']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['coap_resource_value_id']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['coap_resource_value_id']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['coap_resource_value_id']['alter']['external'] = 0;
  $handler->display->display_options['fields']['coap_resource_value_id']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['coap_resource_value_id']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['coap_resource_value_id']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['coap_resource_value_id']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['coap_resource_value_id']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['coap_resource_value_id']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['coap_resource_value_id']['alter']['html'] = 0;
  $handler->display->display_options['fields']['coap_resource_value_id']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['coap_resource_value_id']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['coap_resource_value_id']['hide_empty'] = 0;
  $handler->display->display_options['fields']['coap_resource_value_id']['empty_zero'] = 0;
  /* Field: Coap_resource_value: Name */
  $handler->display->display_options['fields']['payload']['id'] = 'payload';
  $handler->display->display_options['fields']['payload']['table'] = 'coap_resource_value';
  $handler->display->display_options['fields']['payload']['field'] = 'payload';
  $handler->display->display_options['fields']['payload']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['payload']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['payload']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['payload']['alter']['external'] = 0;
  $handler->display->display_options['fields']['payload']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['payload']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['payload']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['payload']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['payload']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['payload']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['payload']['alter']['html'] = 0;
  $handler->display->display_options['fields']['payload']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['payload']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['payload']['hide_empty'] = 0;
  $handler->display->display_options['fields']['payload']['empty_zero'] = 0;
  /* Field: Coap_resource_value: Link */
  $handler->display->display_options['fields']['link_coap_resource_value']['id'] = 'link_coap_resource_value';
  $handler->display->display_options['fields']['link_coap_resource_value']['table'] = 'coap_resource_value';
  $handler->display->display_options['fields']['link_coap_resource_value']['field'] = 'link_coap_resource_value';
  $handler->display->display_options['fields']['link_coap_resource_value']['label'] = 'View';
  $handler->display->display_options['fields']['link_coap_resource_value']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['link_coap_resource_value']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['link_coap_resource_value']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['link_coap_resource_value']['alter']['external'] = 0;
  $handler->display->display_options['fields']['link_coap_resource_value']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['link_coap_resource_value']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['link_coap_resource_value']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['link_coap_resource_value']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['link_coap_resource_value']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['link_coap_resource_value']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['link_coap_resource_value']['alter']['html'] = 0;
  $handler->display->display_options['fields']['link_coap_resource_value']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['link_coap_resource_value']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['link_coap_resource_value']['hide_empty'] = 0;
  $handler->display->display_options['fields']['link_coap_resource_value']['empty_zero'] = 0;
  /* Field: Coap_resource_value: Operations links */
  $handler->display->display_options['fields']['operations']['id'] = 'operations';
  $handler->display->display_options['fields']['operations']['table'] = 'coap_resource_value';
  $handler->display->display_options['fields']['operations']['field'] = 'operations';
  $handler->display->display_options['fields']['operations']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['operations']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['operations']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['operations']['alter']['external'] = 0;
  $handler->display->display_options['fields']['operations']['alter']['replace_spaces'] = 0;
  $handler->display->display_options['fields']['operations']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['operations']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['operations']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['operations']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['operations']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['operations']['alter']['html'] = 0;
  $handler->display->display_options['fields']['operations']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['operations']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['operations']['hide_empty'] = 0;
  $handler->display->display_options['fields']['operations']['empty_zero'] = 0;
  
  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'coap_resource_values_admin_page');
  $handler->display->display_options['path'] = 'admin/content/coap_resource_values/list';
  $handler->display->display_options['menu']['type'] = 'default tab';
  $handler->display->display_options['menu']['title'] = 'List';
  $handler->display->display_options['menu']['weight'] = '-10';
  $handler->display->display_options['tab_options']['type'] = 'tab';
  $handler->display->display_options['tab_options']['title'] = 'Coap_resource_values';
  $handler->display->display_options['tab_options']['description'] = 'Manage coap_resource_values';
  $handler->display->display_options['tab_options']['weight'] = '0';
  $handler->display->display_options['tab_options']['name'] = 'management';
  $translatables['coap_resource_values'] = array(
    t('Master'),
    t('Coap_resource_values'),
    t('more'),
    t('Apply'),
    t('Reset'),
    t('Sort by'),
    t('Asc'),
    t('Desc'),
    t('Items per page'),
    t('- All -'),
    t('Offset'),
    t('Empty '),
    t('No coap_resource_values have been created yet'),
    t('Coap_resource_value ID'),
    t('.'),
    t(','),
    t('Name'),
    t('View'),
    t('Operations links'),
    t('Page'),
  );
  $views[] = $view;
  return $views;

}
