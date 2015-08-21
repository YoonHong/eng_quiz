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
  $content['preface']['#markup'] = t('Quiz History For @name!!!', array('@name' => $GLOBALS['user']->name) );

  $content['table'] = _eng_quiz_view_sq_history();

  return $content;
}

function _eng_quiz_pc_student_qanswer( $sqid, $uid ) {

  $sq_entity = _eng_quiz_get_single_entity( 'eng_quiz_student_qlist' , $sqid );

  $mid = $sq_entity->mid;

  $qmain = _eng_quiz_get_single_entity( 'eng_quiz_main' , $mid );

  $testDate = format_date( $sq_entity->created, 'custom', 'Y-M-d' );

  $content['quiz_info'] = array(
      '#markup' => t('Tite: @title <br/>Test Date : @td',
                    array('@title'=>$qmain->title,
                          '@td'=>$testDate, ) ),
      '#prefix' => '<div>',
      '#suffix' => '</div>',
    );

    // Quiz List
    $slist = _eng_quiz_get_sub_list( $mid );


    // Student Answer List
    $sAList = _eng_quiz_get_student_qlist( $sqid );

    $idx = 0;
    foreach ($slist as $sub) {
      $idx++;

      // Answer List
      $aList = _eng_quiz_get_answer_list( $sub->id );
      $txtAns = "";
      foreach ($aList as $ans) {
        if ( $txtAns ) {
          $txtAns .= " , ";
        }

        $txtAns .= $ans->answer;
      }

      $content['question'.$idx] = array(
        '#markup' => t('Q@idx : @title',
                      array('@idx'=>$idx,
                            '@title'=>$sub->title, ) ),
        '#prefix' => '<div style="padding-top:10px;">',
        '#suffix' => '</div>',
      );

      if( $sub->type == 3 ) {
        // 객관식 문항
        $active = _eng_quiz_get_opt_questions( $sub->id );

        $content['opt'.$idx] = array(
          '#theme' => 'item_list',
          '#items' => $active,
          '#type' => 'ol',
        );

      }

      $content['answer'.$idx] = array(
        '#markup' => t('A@idx : @answer',
                      array('@idx'=>$idx,
                            '@answer'=>$txtAns, )
                      ),
        '#prefix' => '<div>',
        '#suffix' => '</div>',
      );


      $content['user_answer'.$idx] = array(
        '#markup' => t('Your Answer : @answer',
                      array( '@answer'=>$sAList[$idx-1]->answer, )
                      ),
        '#prefix' => '<div>',
        '#suffix' => '</div>',
      );

    }

    $content['link_back'] = array(
      '#markup' => t( '<a href="javascript:window.history.go(-1);">back</a>' ),
      '#prefix' => '<div style="margin-top:20px;">',
      '#suffix' => '</div>',
    );


  return $content;
}

function _eng_quiz_view_sq_history() {
  $content = array();

  $user = $GLOBALS['user'];
  $uid = $user->uid;

  $query = db_select('eng_quiz_student_qlist', 'sq')->extend('PagerDefault');
  $query->join('eng_quiz_main', 'mq', 'mq.id = sq.mid');
  $query->condition('sq.uid', $uid, '=');
  $query->condition('sq.retest', 'N', '=');
  $query->fields('mq',array('title', 'type'))
        ->fields('sq',array('id', 'mid', 'uid', 'cnt', 'created'));
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

      // Create tabular rows for our entities.
      $rows[] = array(
        'data' => array(
          'id' => $entity->mid,
          'title' => l($entity->title, 'eng_quiz/student/'.$entity->id.'/qanswer/'.$uid),
          'cnt' => ''.$entity->cnt.' / '.$total_cnt,
          'created' => format_date( $entity->created, 'custom', 'Y-M-d' ),
        ),
      );
    }
    // Put our entities into a themed table. See theme_table() for details.
    $content['entity_table'] = array(
      '#theme' => 'table',
      '#rows' => $rows,
      '#header' => array(t('ID'), t('Title'), t('Cnt'), t('Test Date')),
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


function _eng_quiz_view_sq_list() {
  $content = array();
  $user = $GLOBALS['user'];
  $uid = $user->uid;

  $query = db_select('eng_quiz_main', 'mq')->extend('PagerDefault');

  $query->leftJoin('eng_quiz_student_qlist', 'sq', "mq.id = sq.mid AND sq.retest = 'N' AND sq.uid = ".$uid);

  $query->fields('mq',array('id', 'title', 'type', 'created'))
        ->fields('sq',array('mid', 'uid'))
        ->condition('mq.useYn', 'Y', '=')
        ->isNull('sq.mid');

  $result = $query->limit(20)
                ->orderBy('mq.id', 'DESC')
                ->execute();

  if ($result->rowCount() > 0 ) {

    foreach ($result as $entity) {
      // Create tabular rows for our entities.
      $rows[] = array(
        'data' => array(
        'id' => $entity->id,
        'title' => l($entity->title, 'eng_quiz/student/'.$entity->id.'/qprocess'),
        'type' => $entity->type,
        'created' => format_date( $entity->created, 'custom', 'Y-M-d' ),
        ),
      );
    }
    // Put our entities into a themed table. See theme_table() for details.
    $content['entity_table'] = array(
      '#theme' => 'table',
      '#rows' => $rows,
      '#header' => array(t('ID'), t('Title'), t('Type'), t('Created Date'),),
    );

    // Attach the pager theme.
    $content['entity_pager'] = array('#theme' => 'pager');

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
      '#markup' => t( 'Q @qNum : <pre>@qTitle</pre>', array( '@qNum' => $idx, '@qTitle' => $sub->title, ) ),
      '#prefix' => '<div style="padding-top:10px;">',
      '#suffix' => '</div>',
    );

    if ( $sub->type == 2) {
      // 주관식
      $form['A'.$idx] = array(
        '#type' => 'textfield',
        '#required' => TRUE,
        '#size' => 130,
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

  $form['link_cancel'] = array(
    '#markup' => t( '<a href="javascript:window.history.go(-1);">back</a>' ),
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


    if ( _eng_quiz_student_check_answer( $sub->id, $form_state['values']['A'.$idx] ) ) {
      $cnt++;
    }

  }

  //$entity2 = _eng_quiz_get_entity( 'eng_quiz_student_qlist', $entity->id);
  $entity->cnt = $cnt;
  drupal_write_record('eng_quiz_student_qlist', $entity, 'id');

  return $form_state['redirect'] = 'eng_quiz/student/quiz/history';
}

function _eng_quiz_student_check_answer( $sid, $sAns ){
  $rv = false;

  $aList = _eng_quiz_get_answer_list( $sid );

  $len = sizeof( $aList );
  for( $i = 0; $i < $len; $i++ ) {
    $a = $aList[ $i ]->answer;
    if( $a == $sAns ) {
      $rv = true;
      break;
    }
  }

  return $rv;
}
