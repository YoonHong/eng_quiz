<?php

function _eng_quiz_pc_main_list_useY( ) {

  return _eng_quiz_pc_main_list( 'Y' );
}

function _eng_quiz_pc_main_list_useN( ) {

  return _eng_quiz_pc_main_list( 'N' );
}

function _eng_quiz_pc_main_list( $yn ) {

  $content['preface']['#markup'] = t('Add Quiz !link.',
      array('!link' => l(t('here'), 'eng_quiz/main/add'))
    );

  $content['table'] = _eng_quiz_pc_main_list_table( $yn );

  return $content;
}

function _eng_quiz_pc_main_list_table( $yn ){
  $content = array();

  $query = db_select('eng_quiz_main', 'd')->extend('PagerDefault');
  $query->fields('d', array('id', 'type', 'title', 'created', 'useYn'));


  $result = $query
    ->condition('d.useYn', $yn, '=')
    ->limit(20)
    ->orderBy('d.id', 'DESC')
    ->execute();

  if ($result->rowCount() > 0 ) {
    $rows = array();
    foreach ($result as $entity) {
      // Create tabular rows for our entities.
      $rows[] = array(
        'data' => array(
        'id' => $entity->id,
        'title' => l($entity->title, 'eng_quiz/sub/' . $entity->id ),
        'type' => $entity->type,
        'created' => format_date( $entity->created, 'custom', 'Y-M-d' ),
        'used' => $entity->useYn,
        'edit' => l(t('edit'), 'eng_quiz/main/'.$entity->id.'/edit'),
        'del' => l(t('del'), 'eng_quiz/main/'.$entity->id.'/del'),
        ),
      );

    }

    // Put our entities into a themed table. See theme_table() for details.
    $content['entity_table'] = array(
      '#theme' => 'table',
      '#rows' => $rows,
      '#header' => array(t('ID'), t('Title'), t('Type'), t('Created Date'), t('Use'), t('Edit'), t('Del') ),
      '#empty' => t('No Quiz!!!'),
    );

    // Attach the pager theme.
    $content['entity_pager'] = array('#theme' => 'pager');

  } else {
    $content[] = array(
      '#type' => 'item',
      '#markup' => t('No Quiz!!!'),
    );
  }

  return $content;
}

function _eng_quiz_pc_main_add() {
  $entity = entity_create('eng_quiz_main', array('id' => null));
  return drupal_get_form('_eng_quiz_main_add_form', $entity);
}

function _eng_quiz_pc_main_edit( $id ) {
  $entity = _eng_quiz_get_single_entity('eng_quiz_main', $id);
	return drupal_get_form('_eng_quiz_main_add_form', $entity);
}

function _eng_quiz_pc_main_del( $id ) {
  $entity = _eng_quiz_get_single_entity('eng_quiz_main', $id);
  return drupal_get_form('_eng_quiz_main_del_form', $entity);
}

function _eng_quiz_main_add_form($form, &$form_state, $eng_quiz_main = NULL) {

  $form['basic_entity'] = array(
    '#type' => 'value',
    '#value' => $eng_quiz_main,
  );

  $form['title'] = array(
    '#title' => t('Quiz Title'),
    '#type' => 'textfield',
    '#default_value' => isset($eng_quiz_main->title) ? $eng_quiz_main->title : '',
    '#required' => TRUE,
  );

  $form['type'] = array(
    '#title' => t('Quiz Type'),
    '#type' => 'textfield',
    '#default_value' => isset($eng_quiz_main->type) ? $eng_quiz_main->type : '',
    '#required' => TRUE,
  );

  if ( isset($eng_quiz_main->useYn) ) {
    $form['useYn'] = array(
      '#title' => t('Use : Yes or No?'),
      '#type' => 'select',
      '#options' => array( 'Y' => 'Yes', 'N' => 'No', ),
      '#default_value' => $eng_quiz_main->useYn,

    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => isset($eng_quiz_main->id) ? t('Update') : t('Save'),
  );

  $form['link_cancel'] = array(
    '#markup' => t( '<a href="javascript:window.history.go(-1);">back</a>' ),
  );

  return $form;
}

function _eng_quiz_main_add_form_validate($form, &$form_state) {
  //dpm( format_date(REQUEST_TIME, 'custom', 'Y-m-d' ) ) ;
  //form_set_error('title', t('Le champ courriel est invalide.'));
}

function _eng_quiz_main_add_form_submit($form, &$form_state) {
  $org_entity = $form_state['values']['basic_entity'];

  if( $org_entity->id == null ) {
  	// Insert
  	$entity = entity_create('eng_quiz_main', array('id' => null));
  } else {
    // Edit
  	$entity = entity_create('eng_quiz_main', array('id' => $org_entity->id));
  }

  $entity->title  = $form_state['values']['title'];
  $entity->type  = $form_state['values']['type'];

  if( $org_entity->id == null ) {
  	// Insert
    $entity->created  = REQUEST_TIME;

    drupal_write_record('eng_quiz_main', $entity);
  } else {
    // Edit
    $entity->useYn  = $form_state['values']['useYn'];
    $entity->changed  = REQUEST_TIME;

  	drupal_write_record('eng_quiz_main', $entity, 'id');
  }

  $form_state['redirect'] = 'eng_quiz/main/list';

}

function _eng_quiz_main_del_form($form, &$form_state, $entity) {
  $form['basic_entity'] = array(
    '#type' => 'value',
    '#value' => $entity,
  );

  $question = t( "@title : Are you sure you want to delete this Quiz?"
                , array('@title' => $entity->title,) );
  $path = 'eng_quiz/main/list';
  $description = t( "This cannot be undone." );
  $yes = t( "Confirm" );
  $no = t( "Cancel" );
  return confirm_form( $form, $question, $path, $description, $yes, $no );
}

function _eng_quiz_main_del_form_submit($form, &$form_state) {
  $org_entity = $form_state['values']['basic_entity'];

  db_delete('eng_quiz_main')
    ->condition('id', $org_entity->id)
    ->execute();

  $msg = t( "The Quiz ( @title ) has been deleted."
    , array('@title' => $org_entity->title,) );
  drupal_set_message( $msg );

  $form_state['redirect'] = 'eng_quiz/main/list';
}
