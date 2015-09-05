<?php

function _eng_quiz_pc_admin_quiz_list() {

//  $content['preface']['#markup'] = t( 'Student Quiz List' );

  $content['table'] = _eng_quiz_admin_ql_table( 'latest' );

  return $content;
}

function _eng_quiz_pc_admin_list_by_quiz() {
  $content['table'] = _eng_quiz_admin_aList_byQuiz_table( );

  return $content;
}

function _eng_quiz_pc_admin_list_by_student() {

  $content['table'] = _eng_quiz_admin_aList_byStudent_table( );

  return $content;
}

function _eng_quiz_pc_admin_list_by_quiz_sub( $id ) {
  $content['table'] = _eng_quiz_admin_ql_table( 'byquiz', $id );

  $content['link_back'] = array(
    '#markup' => l('back', 'eng_quiz/admin/quiz'),
    '#prefix' => '<div style="margin-top:20px;">',
    '#suffix' => '</div>',
  );

  return $content;
}

function _eng_quiz_pc_admin_list_by_student_sub( $id ) {
  $content['table'] = _eng_quiz_admin_ql_table( 'bystudent', $id );

  $content['link_back'] = array(
    '#markup' => l('back', 'eng_quiz/admin/student'),
    '#prefix' => '<div style="margin-top:20px;">',
    '#suffix' => '</div>',
  );

  return $content;
}

function _eng_quiz_admin_ql_table( $type, $tid = '' ) {
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

  if ( $type == 'byquiz' ) {
    $query->condition('mq.id', $tid, '=');
  } else if ( $type == 'bystudent' ) {
    $query->condition('user.uid', $tid, '=');
  }

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

      if ( isset($profile->field_full_name) && count($profile->field_full_name) > 0 ) {
        $rName = $profile->field_full_name['und'][0]['value'];
      } else {
        $rName = '';
      }


      if ( $type == "bystudent" ) {
        $optId = $tid;
      } else {
        $optId = $entity->mid;
      }

      // Create tabular rows for our entities.
      $rows[] = array(
        'data' => array(
          'uid' => $entity->name,
          'name' => $rName,
          'title' => l($entity->title, 'eng_quiz/student/'.$entity->id.'/qanswer/'.$uid),
          'cnt' => ''.$entity->cnt.' / '.$total_cnt,
          'created' => format_date( $entity->created, 'custom', 'Y-M-d' ),
          'retest' => l(t('ReTest'), 'eng_quiz/admin/'.$entity->id.'/'.$type.'/retest/'.$optId),
          'del' => l(t('x'), 'eng_quiz/admin/'.$entity->id.'/'.$type.'/del/'.$optId),
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
      '#markup' => t('No Item!!!'),
    );
  }

  return $content;

}

function _eng_quiz_admin_aList_byQuiz_table( ){
  $content = array();

  $query = db_select('eng_quiz_main', 'd')->extend('PagerDefault');
  $query->fields('d', array('id', 'type', 'title', 'created', 'useYn'));


  $result = $query
    //->condition('d.useYn', $yn, '=')
    ->limit(20)
    ->orderBy('d.id', 'DESC')
    ->execute();

  if ($result->rowCount() > 0 ) {
    $rows = array();
    foreach ($result as $entity) {
      // Create tabular rows for our entities.

      $rcnt_query = db_query(
          "SELECT COUNT(*) rcnt FROM {eng_quiz_student_qlist} WHERE mid = :mid AND retest = 'N' ", array(':mid' => $entity->id,)
       );
      $result = $rcnt_query->fetchObject();
      $rcnt = '';
      if( $result->rcnt > 0 ) {
        $rcnt = l( $result->rcnt.' ( View List )' , 'eng_quiz/admin/quiz/'.$entity->id );
      } else {
        $rcnt = '0';
      }

      $rows[] = array(
        'data' => array(
        'id' => $entity->id,
        'title' => $entity->title,
        'type' => $entity->type,
        'rcnt' => $rcnt,
        'created' => format_date( $entity->created, 'custom', 'Y-M-d' ),
        'used' => $entity->useYn,
        ),
      );

    }

    // Put our entities into a themed table. See theme_table() for details.
    $content['entity_table'] = array(
      '#theme' => 'table',
      '#rows' => $rows,
      '#header' => array(t('ID'), t('Title'), t('Type'), t('Result Count'), t('Created Date'), t('Use'), ),
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

function _eng_quiz_admin_aList_byStudent_table( ){
  $content = array();

  $query = db_select('users', 'u')->extend('PagerDefault');
  $query->leftJoin('users_roles', 'r', 'u.uid = r.uid');
  $query->fields('u', array('uid', 'name', 'mail', 'created', 'status'));
  $query->fields('r', array('rid',));


  $result = $query
    ->condition('u.status', 1, '=')
    ->isNull('r.rid')
    ->limit(20)
    ->execute();

  if ($result->rowCount() > 0 ) {
    $rows = array();
    foreach ($result as $entity) {
      // Create tabular rows for our entities.

      $rcnt = 0;

      $rcnt_query = db_query(
          "SELECT COUNT(*) rcnt FROM {eng_quiz_student_qlist} WHERE uid = :uid AND retest = 'N' ", array(':uid' => $entity->uid,)
       );
      $result = $rcnt_query->fetchObject();

      if( $result->rcnt > 0 ) {
        $rcnt = l( $result->rcnt.' ( View List )' , 'eng_quiz/admin/student/'.$entity->uid );
      } else {
        $rcnt = '0';
      }


      $rows[] = array(
        'data' => array(
        'uid' => $entity->name,
        'name' => _eng_quiz_get_user_name($entity->uid),
        'rid' => $rcnt,
        ),
      );

    }

    // Put our entities into a themed table. See theme_table() for details.
    $content['entity_table'] = array(
      '#theme' => 'table',
      '#rows' => $rows,
      '#header' => array(t('User ID'), t('User Name'), t('Result Count'), ),
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

function _eng_quiz_pc_admin_latest_del( $id, $mid ) {
  return drupal_get_form('_eng_quiz_pc_admin_quiz_del_form', $id, 'latest', $mid );
}

function _eng_quiz_pc_admin_byquiz_del( $id, $mid ) {
  return drupal_get_form('_eng_quiz_pc_admin_quiz_del_form', $id, 'byquiz', $mid );
}

function _eng_quiz_pc_admin_bystudent_del( $id, $mid ) {
  return drupal_get_form('_eng_quiz_pc_admin_quiz_del_form', $id, 'bystudent', $mid );
}

function _eng_quiz_pc_admin_quiz_del_form($form, &$form_state, $id, $type, $mid) {
  $form['delid'] = array(
    '#type' => 'value',
    '#value' => $id,
  );

  $form['mid'] = array(
    '#type' => 'value',
    '#value' => $mid,
  );

  $form['rtype'] = array(
    '#type' => 'value',
    '#value' => $type,
  );

  $form['question'] = array(
      '#markup' => t( "Do you want to delete this Quiz Result?" ),
      '#prefix' => '<h1 class=title>',
      '#suffix' => '</h1>',
    );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Confirm'),
  );

  $form['link_cancel'] = array(
    '#markup' => t( '<a href="javascript:window.history.go(-1);">Cancel</a>' ),
  );

  return $form;
}


function _eng_quiz_pc_admin_quiz_del_form_submit($form, &$form_state) {
  $did = $form_state['values']['delid'];
  $rtype = $form_state['values']['rtype'];
  $mid = $form_state['values']['mid'];
  $redirect = 'eng_quiz/admin/latest';

  if( $rtype == 'byquiz' ) {
    $redirect = 'eng_quiz/admin/quiz/'.$mid;
  } else if( $rtype == 'bystudent' ) {
    $redirect = 'eng_quiz/admin/student/'.$mid;
  }

  db_delete('eng_quiz_student_qlist')
    ->condition('id', $did)
    ->execute();

  $msg = t( "The Quiz Result has been deleted." );
  drupal_set_message( $msg );

  $form_state['redirect'] = $redirect;
}

function _eng_quiz_pc_admin_latest_retest( $id, $mid ) {
  return drupal_get_form('_eng_quiz_pc_admin_retest_form', $id, 'latest', $mid);
}

function _eng_quiz_pc_admin_byquiz_retest( $id, $mid ) {
  return drupal_get_form('_eng_quiz_pc_admin_retest_form', $id, 'byquiz', $mid);
}

function _eng_quiz_pc_admin_bystudent_retest( $id, $mid ) {
  return drupal_get_form('_eng_quiz_pc_admin_retest_form', $id, 'bystudent', $mid);
}

function _eng_quiz_pc_admin_retest_form($form, &$form_state, $id, $type, $mid) {
  $form['rid'] = array(
    '#type' => 'value',
    '#value' => $id,
  );

  $form['rtype'] = array(
    '#type' => 'value',
    '#value' => $type,
  );

  $form['mid'] = array(
    '#type' => 'value',
    '#value' => $mid,
  );

  $form['question'] = array(
      '#markup' => t( "Do you want to make this Quiz ReTested?" ),
      '#prefix' => '<h1 class=title>',
      '#suffix' => '</h1>',
    );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Confirm'),
  );

  $form['link_cancel'] = array(
    '#markup' => t( '<a href="javascript:window.history.go(-1);">Cancel</a>' ),
  );

  return $form;
}

function _eng_quiz_pc_admin_retest_form_submit($form, &$form_state) {
  $rid = $form_state['values']['rid'];
  $rtype = $form_state['values']['rtype'];
  $mid = $form_state['values']['mid'];
  $redirect = 'eng_quiz/admin/latest';

  if( $rtype == 'byquiz' ) {
    $redirect = 'eng_quiz/admin/quiz/'.$mid;
  } else if( $rtype == 'bystudent' ) {
    $redirect = 'eng_quiz/admin/student/'.$mid;
  }

  db_update('eng_quiz_student_qlist')
    ->fields(array(
      'retest' => 'Y',
    ))
    ->condition('id', $rid, '=')
    ->execute();

  $msg = t( "Completed!" );
  drupal_set_message( $msg );

  $form_state['redirect'] = $redirect;
}
