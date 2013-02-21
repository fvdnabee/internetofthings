<?php

/**
 * Implements hook_menu().
 */
function page_example_menu() {
  $items = array();

  // Submissions listing. (content -> nieuwe tab)
  $items['admin/content/page_example'] = array(
    'title' => 'page_examples',
    'access callback' => 'user_access',
    'access arguments' => array('access all page_example results'),
    'description' => 'View and edit all the available page_examples on your site.',
    'type' => MENU_LOCAL_TASK,
  );

  // Admin Settings. (configuration-> nieuwe entry)
  $items['admin/config/content/page_example'] = array(
    'title' => 'page_example settings',
    //'access callback' => 'user_access', //dit hoeft er niet bij
    'access arguments' => array('administer site configuration'), //dit moet erbij
    'description' => 'Global configuration of page_example functionality.',
    'type' => MENU_NORMAL_ITEM,
  );
  
  //(pagina's die goed te keuren zijn door administrator)
  $items['example'] = array(
    'title' => 'Example Page',
    'page callback' => 'example_page',
    'access arguments' => array('access content'),
    'type' => MENU_SUGGESTED_ITEM,
  );
  
  // Make "Foo settings" appear on the admin Config page
  $items['admin/config/system/foo'] = array(
    'title' => 'Foo settings',
    'access arguments' => array('access content'),
    'type' => MENU_NORMAL_ITEM,
  );
  // Make "Tab 1" the main tab on the "Foo settings" page
  $items['admin/config/system/foo/tab1'] = array(
    'title' => 'Tab 1',
    'type' => MENU_DEFAULT_LOCAL_TASK,
    // Access callback, page callback, and theme callback will be inherited
    // from 'admin/config/system/foo', if not specified here to override.
  );
  // Make an additional tab called "Tab 2" on "Foo settings"
  $items['admin/config/system/foo/tab2'] = array(
    'title' => 'Tab 2',
    'type' => MENU_LOCAL_TASK,
    // Page callback and theme callback will be inherited from
    // 'admin/config/system/foo', if not specified here to override.
    // Need to add access callback or access arguments.
  );
  
  return $items;
}