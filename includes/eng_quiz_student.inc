<?php

function _eng_quiz_pc_student_quiz() {
  $content['preface']['#markup'] = t('Select a Quiz.');

  $content['table'] = _eng_quiz_view_sq_list();

  return $content;
}

function _eng_quiz_pc_student_qprocess( $mid ) {
  return drupal_get_form('_eng_quiz_student_qprocess_form', $mid);
}

function _eng_quiz_pc_student_quiz_history() {
  $content['preface']['#markup'] = t('Quiz History!!!'.$GLOBALS['user']->name);

  $content['table'] = _eng_quiz_view_sq_history();

  return $content;
}

function _eng_quiz_view_sq_history() {
  $content = array();

  $user = $GLOBALS['user'];

  // Load all of our entities.
  $result = db_query(
      'SELECT A.id, A.mid, A.cnt
        FROM {eng_quiz_student_qlist} A
        WHERE 1=1
        AND A.uid = :uid', array(':uid' => $user->uid));

  if ($result->rowCount() > 0 ) {

    foreach ($result as $entity) {
      // Get Cnt of Sub Quiz]
      $total_cnt = db_query(
          'SELECT COUNT(*)
            FROM {eng_quiz_sub} A
            WHERE 1=1
            AND A.mid = :mid', array(':mid' => $entity->mid)
          )->fetchField();

      // Create tabular rows for our entities.
      $rows[] = array(
        'data' => array(
          'id' => $entity->id,
        'title' => l($entity->mid, 'eng_quiz/student/'.$entity->id.'/qprocess'),
          'cnt' => ''.$entity->cnt.' / '.$total_cnt,
        ),
      );
    }
    // Put our entities into a themed table. See theme_table() for details.
    $content['entity_table'] = array(
      '#theme' => 'table',
      '#rows' => $rows,
      '#header' => array(t('ID'), t('Title'), t('Cnt'),),
    );
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


function _eng_quiz_view_sq_list() {
  $content = array();

  // Load all of our entities.
  $result = db_query(
      'SELECT A.id, A.title, A.type
        FROM {eng_quiz_main} A
        WHERE 1=1
          AND A.id NOT IN ( SELECT mid FROM {eng_quiz_student_qlist} )'
      );

  if ($result->rowCount() > 0 ) {

    foreach ($result as $entity) {
      // Create tabular rows for our entities.
      $rows[] = array(
        'data' => array(
          'id' => $entity->id,
        'title' => l($entity->title, 'eng_quiz/student/'.$entity->id.'/qprocess'),
          'type' => $entity->type,
        ),
      );
    }
    // Put our entities into a themed table. See theme_table() for details.
    $content['entity_table'] = array(
      '#theme' => 'table',
      '#rows' => $rows,
      '#header' => array(t('ID'), t('Title'), t('Type'),),
    );
  }
  else {
    // There were no entities. Tell the user.
    $content[] = array(
      '#type' => 'item',
      '#markup' => t('No Quiz!!!'),
    );
  }

  return $content;
}

function _eng_quiz_student_qprocess_form($form, &$form_state, $mid) {

  $form['v_mid'] = array(
    '#type' => 'value',
    '#value' => $mid,
  );

  $form['mQuiz'] = array(
    '#type' => 'markup',
    '#markup' => t(
          '
            <p>Title : @title
            <br/>Type : @type</p>

          '
        , array('@title' => 't1', '@type' => 't2', )),
  );


  $slist = _eng_quiz_get_sub_list( $mid );

  $idx = 0;
  foreach ($slist as $sub) {
    $idx++;

    $form['subList'.$idx] = array(
      '#type' => 'markup',
      '#markup' => '<div>Q '.$idx.' : '.$sub->title.'</div>',
    );

    if ( $sub->type == 2) {
      // 주관식
      $form['A'.$idx] = array(
        '#type' => 'textfield',
        '#required' => TRUE,
      );

    } else if ( $sub->type == 3 ) {
      // 객관식
//      $active = array(0 => t('Closed'), 1 => t('Active'));
      $active = _eng_quiz_get_opt_questions( $sub->id );

      $form['A'.$idx] = array(
        '#type' => 'radios',
        '#required' => TRUE,
        '#options' => $active,
      );

    }

  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit!'),
  //  '#weight' => 50,
  );

  return $form;

}

function _eng_quiz_get_opt_questions( $sid ){
  $opts = _eng_quiz_get_opt_list( $sid );

  $active = array();
  $idx = 0;
  foreach($opts as $opt) {
    $idx++;
    $active[$idx] = $opt->title;
  }

  return $active;
}

function _eng_quiz_student_qprocess_form_submit($form, &$form_state) {

  $user = $GLOBALS['user'];
  $mid = $form_state['values']['v_mid'];

  // Insert Data to eng_quiz_student_qlist

  $entity = entity_create('eng_quiz_student_qlist', array('id' => null));
  $entity->uid = $user->uid;
  $entity->mid = $mid;
  $entity->cnt = 0;
  $entity->created = REQUEST_TIME;

  drupal_write_record('eng_quiz_student_qlist', $entity);

  $slist = _eng_quiz_get_sub_list( $mid );
  $idx = 0;
  $cnt = 0;
  foreach ($slist as $sub) {
    $idx++;
    $data = array(
      'sid' => $sub->id,
      'sqid' => $entity->id,
      'answer' => $form_state['values']['A'.$idx],
    );
    drupal_write_record('eng_quiz_student_qlist_dtl', $data);

    if( $sub->answer == $form_state['values']['A'.$idx] ) {

      $cnt++;
    }

  }

  //$entity2 = _eng_quiz_get_entity( 'eng_quiz_student_qlist', $entity->id);
  $entity->cnt = $cnt;
  drupal_write_record('eng_quiz_student_qlist', $entity, 'id');

  return $form_state['redirect'] = 'eng_quiz/student/quiz/history';
}
