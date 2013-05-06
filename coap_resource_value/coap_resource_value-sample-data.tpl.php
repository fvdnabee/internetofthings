<?php

/**
 * @file
 * Example tpl file for theming a single coap_resource_value-specific theme
 *
 * Available variables:
 * - $status: The variable to theme (while only show if you tick status)
 * 
 * Helper variables:
 * - $coap_resource_value: The coap_resource_value object this status is derived from
 */
?>

<div class="coap_resource_value-status">
  <?php print '<strong>coap_resource_value Sample Data:</strong> ' . $coap_resource_value_sample_data = ($coap_resource_value_sample_data) ? 'Switch On' : 'Switch Off' ?>
</div>