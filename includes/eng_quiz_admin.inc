<?php

function _eng_quiz_pc_admin_quiz_list() {

//  $content['preface']['#markup'] = t( 'Student Quiz List' );

  $content['table'] = _eng_quiz_admin_ql_table();

  return $content;
}

function _eng_quiz_admin_ql_table() {
  $content = array();

  $user = $GLOBALS['user'];
  $uid = $user->uid;

  $query = db_select('eng_quiz_student_qlist', 'sq')->extend('PagerDefault');
  $query->join('eng_quiz_main', 'mq', 'mq.id = sq.mid');
  $query->join('users', 'user', 'user.uid = sq.uid');

  $query->fields('mq',array('title', 'type'))
        ->fields('user',array('name'))
        ->fields('sq',array('id', 'mid', 'uid', 'cnt', 'created'))
        ->condition('sq.retest', 'N', '=');
  $query->limit(20)
        ->orderBy('sq.created', 'DESC');

  $result = $query->execute();

  if ($result->rowCount() > 0 ) {

    foreach ($result as $entity) {
      // Get Cnt of Sub Quiz]
      $total_cnt = db_query(
          'SELECT COUNT(*)
            FROM {eng_quiz_sub} A
            WHERE 1=1
            AND A.mid = :mid', array(':mid' => $entity->mid)
          )->fetchField();

      $profile=profile2_load_by_user($entity->uid,'main');
      $rName = count($profile->field_full_name) > 0 ? $profile->field_full_name['und'][0]['value'] : '';

      // Create tabular rows for our entities.
      $rows[] = array(
        'data' => array(
          'uid' => $entity->name,
          'name' => $rName,
          'title' => l($entity->title, 'eng_quiz/student/'.$entity->id.'/qanswer/'.$uid),
          'cnt' => ''.$entity->cnt.' / '.$total_cnt,
          'created' => format_date( $entity->created, 'custom', 'Y-M-d' ),
          'retest' => l(t('ReTest'), 'eng_quiz/admin/'.$entity->id.'/latest/retest'),
          'del' => l(t('x'), 'eng_quiz/admin/'.$entity->id.'/latest/del'),
        ),
      );
    }
    // Put our entities into a themed table. See theme_table() for details.
    $content['entity_table'] = array(
      '#theme' => 'table',
      '#rows' => $rows,
      '#header' => array( t('User ID'), t('User Name'), t('Quiz Title'), t('Result'), t('Test Date'), t('ReTest'), t('Del'), ),
    );

    // Attach the pager theme.
    $content['entity_pager'] = array('#theme' => 'pager');
  }
  else {
    // There were no entities. Tell the user.
    $content[] = array(
      '#type' => 'item',
      '#markup' => t('No History!!!'),
    );
  }

  return $content;

}

function _eng_quiz_pc_admin_latest_del( $id ) {
  return drupal_get_form('_eng_quiz_pc_admin_latest_del_form', $id);
}

function _eng_quiz_pc_admin_latest_del_form($form, &$form_state, $id) {
  $form['delid'] = array(
    '#type' => 'value',
    '#value' => $id,
  );

  $question = t( "Do you want to delete this Quiz Result?" );
  $path = 'eng_quiz/admin/latest';
  $description = t( "This cannot be undone." );
  $yes = t( "Confirm" );
  $no = t( "Cancel" );
  return confirm_form( $form, $question, $path, $description, $yes, $no );
}


function _eng_quiz_pc_admin_latest_del_form_submit($form, &$form_state) {
  $did = $form_state['values']['delid'];

  db_delete('eng_quiz_student_qlist')
    ->condition('id', $did)
    ->execute();

  $msg = t( "The Quiz Result has been deleted." );
  drupal_set_message( $msg );

  $form_state['redirect'] = 'eng_quiz/admin/latest';
}

function _eng_quiz_pc_admin_latest_retest( $id ) {
  return drupal_get_form('_eng_quiz_pc_admin_latest_retest_form', $id);
}

function _eng_quiz_pc_admin_latest_retest_form($form, &$form_state, $id) {
  $form['rid'] = array(
    '#type' => 'value',
    '#value' => $id,
  );

  $question = t( "Do you want to make this Quiz ReTested?" );
  $path = 'eng_quiz/admin/latest';
  $description = t( "This cannot be undone." );
  $yes = t( "Confirm" );
  $no = t( "Cancel" );
  return confirm_form( $form, $question, $path, $description, $yes, $no );
}


function _eng_quiz_pc_admin_latest_retest_form_submit($form, &$form_state) {
  $rid = $form_state['values']['rid'];

  db_update('eng_quiz_student_qlist')
    ->fields(array(
      'retest' => 'Y',
    ))
    ->condition('id', $rid, '=')
    ->execute();

  $msg = t( "Completed!" );
  drupal_set_message( $msg );

  $form_state['redirect'] = 'eng_quiz/admin/latest';
}
