<?php

/**
 * This field handler aggregates operations that can be done on a coap_resource_value
 * under a single field providing a more flexible way to present them in a view
 */
class coap_resource_value_handler_coap_resource_value_operations_field extends views_handler_field {
  function construct() {
    parent::construct();

    $this->additional_fields['coap_resource_value_id'] = 'coap_resource_value_id';
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {

    $links = menu_contextual_links('coap_resource_value', 'admin/content/coap_resource_values/coap_resource_value', array($this->get_value($values, 'coap_resource_value_id')));
    
    if (!empty($links)) {
      return theme('links', array('links' => $links, 'attributes' => array('class' => array('links', 'inline', 'operations'))));
    }
  }
}
