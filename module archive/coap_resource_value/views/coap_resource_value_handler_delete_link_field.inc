<?php

/**
 * @file
 * Contains a Views field handler to take care of displaying deletes links
 * as fields
 */


class coap_resource_value_handler_delete_link_field extends coap_resource_value_handler_link_field {
  function construct() {
    parent::construct();
    $this->additional_fields['type'] = 'type';
  }


  function render($values) {
    $type = $values->{$this->aliases['type']};
    
    //Creating a dummy coap_resource_value to check access against
    $dummy_coap_resource_value = (object) array('type' => $type);
    if (!coap_resource_value_access('edit', $dummy_coap_resource_value)) {
      return;
    }
    
    $text = !empty($this->options['text']) ? $this->options['text'] : t('delete');
    $coap_resource_value_id = $values->{$this->aliases['coap_resource_value_id']};
    
    return l($text, 'admin/content/coap_resource_values/coap_resource_value/' . $coap_resource_value_id . '/delete');
  }
}
