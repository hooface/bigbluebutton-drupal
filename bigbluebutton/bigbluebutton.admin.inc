<?php
/* 
Plugin Name: BigBlueButton BN
Plugin URI: http://blindsidenetworks.com/integration
Author: Blindside Networks
Author URI: http://blindsidenetworks.com/
Description: BigBlueButton is an open source web conferencing system. This module integrates BigBlueButton into Drupal allowing website administrators to create and manage meetings rooms that can be used by visitors and registred users. For more information on setting up your own BigBlueButton server or for using an external hosting provider visit http://bigbluebutton.org/support

   Copyright 2011 Blindside Networks

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Versions:
   7.x-1.0.x-dev	--  	Initial version written by Jesus Federico
                    		(email : jesus [a t ] b l i n d si de n  et w o rks DOT ca)
   
*/

/**
* @file
* Administration page callbacks for the bigbluebutton module.
*/

require_once('bbb_api/bbb_api.php');

/**
* BigBlueButton: General Settings.
*
*/
function bigbluebutton_admin_settings() {

	$form['bigbluebutton_server_url'] = array(
		'#type' => 'textfield',
		'#title' => t('URL of BigBlueButton server'),
		'#description' => t('Enter the URL to access the BigBlueButton server, eg. "http://example.com/bigbluebutton/"'),
		'#default_value' => variable_get('bigbluebutton_server_url', ''),
		'#size' => 45
		);

	$form['bigbluebutton_server_salt'] = array(
		'#type' => 'textfield',
		'#title' => t('Salt of BigBlueButton server'),
		'#description' => t('Enter the Salt of the BigBlueButton server. It can be found in /var/lib/tomcat6/webapps/bigbluebutton/WEB-INF/classes/bigbluebutton.properties'),
		'#default_value' => variable_get('bigbluebutton_server_salt', ''),
		'#size' => 45
		);

	$form['#submit'][] = 'bigbluebutton_admin_settings_submit';

	return system_settings_form($form);
}

/**
 * BigBlueButton: Meeting room form.
 */
function bigbluebutton_admin_meeting_room_form($form, &$form_state, $op = 'add', $meeting_room_id = NULL ) {

	// If this is a new meeting room, the function will return an empty $meeting_room.
	$meeting_room = bigbluebutton_getMeeting_room($meeting_room_id);

  	$form['meeting_name'] = array(
    	'#type' => 'textfield',
    	'#title' => t('Meeting room'),
    	'#maxlength' => 35,
    	'#default_value' => $meeting_room['meeting_name'],
    	'#description' => t("Example: 'Demo Meeting'."),
    	'#required' => TRUE,
  		);

  	$form['attendee_pass'] = array(
    	'#type' => 'textfield',
    	'#title' => t('Attendee password'),
    	'#maxlength' => 25,
    	'#default_value' => $meeting_room['attendee_pass'],
    	'#description' => t("Example: 'ap' . To specify the password regular attendees should use to access the room."),
    	'#required' => TRUE,
  		);

  	$form['moderator_pass'] = array(
    	'#type' => 'textfield',
    	'#title' => t('Moderator password'),
    	'#maxlength' => 25,
    	'#default_value' => $meeting_room['moderator_pass'],
    	'#description' => t("Example: 'mp' . To specify the password moderators should use to access the room. Moderators have access to the presentation module and features to control the meeting."),
    	'#required' => TRUE,
  		);

  	$form['wait_for_moderator'] = array(
    	'#type' => 'checkbox',
    	'#title' => t('Wait for moderator to start meeting.'),
    	'#default_value' => $meeting_room['wait_for_moderator'],
    	'#description'   => t('Checking this box it will be required a moderator in the meeting in order to allow attendees get into.'),
  		);

  	$form['newwindow'] = array(
    	'#type' => 'checkbox',
    	'#title' => t('Open meeting in a new window.'),
    	'#default_value' => $meeting_room['newwindow'],
    	'#description'   => t('Checking this box it will open a new window to join meetings.'),
  		);

  	$form['record'] = array(
    	'#type' => 'checkbox',
    	'#title' => t('Playback recording for meeting on/off.'),
    	'#default_value' => $meeting_room['record'],
    	'#description'   => t('Checking this box it will activate the playback recording for the current meeting.'),
  		);

  	$form['description'] = array(
    	'#type' => 'textfield',
    	'#title' => t('Description for the playback recording.'),
    	'#maxlength' => 64,
    	'#default_value' => $meeting_room['description'],
    	'#description'   => t('Description for the play back recording related to the current meeting.'),
  		);

  	$form['welcome'] = array(
    	'#type' => 'textarea',
    	'#title' => t('Welcome message.'),
    	'#default_value' => $meeting_room['welcome'],
    	'#description'   => t('Message to be displayed in the chat window as welcoming once the user logs in.'),
  		);

        $form['actions'] = array('#type' => 'actions');
  	$form['actions']['submit'] = array(
    	'#type' => 'submit',
    	'#value' => t('Submit'),
  		);

  	return $form;

}

/**
 * Process the contact category edit page form submission.
 */
function bigbluebutton_admin_meeting_room_form_submit($form, &$form_state) {

    if ( $form_state['build_info']['args'][0] == 'add' ) {
        db_insert('meeting_room')
            ->fields(array(
                'meeting_id' => BigBlueButton::_rand_string( 16 ),
                'meeting_name' => $form_state['values']['meeting_name'],
                'meeting_version' => time(),
                'attendee_pass' => $form_state['values']['attendee_pass'],
                'moderator_pass' => $form_state['values']['moderator_pass'],
                'wait_for_moderator' => $form_state['values']['wait_for_moderator'],
                'newwindow' => $form_state['values']['newwindow'],
                'record' => $form_state['values']['record'],
                'description' => $form_state['values']['description'],
                'welcome' => $form_state['values']['welcome'],
                ))
            ->execute();
    } else {
    	db_update('meeting_room')
            ->condition('mid', $form_state['build_info']['args'][1])
            ->fields(array(
      		'meeting_name' => $form_state['values']['meeting_name'],
      		'attendee_pass' => $form_state['values']['attendee_pass'],
      		'moderator_pass' => $form_state['values']['moderator_pass'],
      		'wait_for_moderator' => $form_state['values']['wait_for_moderator'],
                'newwindow' => $form_state['values']['newwindow'],
                'record' => $form_state['values']['record'],
                'description' => $form_state['values']['description'],
                'welcome' => $form_state['values']['welcome'],
    		))
            ->execute();
    }
	
    drupal_set_message(t('Meeting room %meeting_name has been saved.', array('%meeting_name' => $form_state['values']['meeting_name'])));
    $form_state['redirect'] = 'admin/config/system/bigbluebutton/meeting_room';

}


/**
 * @file
 * Defines functions for administrative interface for meeting rooms
 *
 */

/**
 * Meeting rooms/list tab.
 */
function bigbluebutton_admin_meeting_room_list() {
	$url_val = variable_get('bigbluebutton_server_url', '');
	$salt_val = variable_get('bigbluebutton_server_salt', '');

	// Initialize table headers
  	$header = array(
    	t('Meeting room'),
    	t('Attendee password'),
    	t('Moderator password'),
    	t('Status'),
    	array('data' => t('Operations'), 'colspan' => 3),
  		);
  	// Initialize rows array
	$rows = array();

  	// Get all the meeting rooms from the database.
	$meeting_rooms = db_select('meeting_room', 'm')
    	->fields('m', array('mid', 'meeting_id', 'meeting_name', 'attendee_pass', 'moderator_pass', 'wait_for_moderator'))
    	->orderBy('meeting_name')
    	->execute()
    	->fetchAll();

  	// Loop through the meeting rooms and add them to the table.
  	foreach ($meeting_rooms as $meeting_room) {
	    //debug(BigBlueButtonBN::isMeetingRunning((string) $meeting_room->meeting_name, $url_val, $salt_val), "status", $print_r = TRUE);
		
    	$rows[] = array(
      		check_plain($meeting_room->meeting_name),
      		check_plain($meeting_room->attendee_pass),
      		check_plain($meeting_room->moderator_pass),
      		//(BigBlueButtonBN::isMeetingRunning((string) $meeting_room->meeting_name, $url_val, $salt_val) ? "Running" : "Not running"),
      		(string) BigBlueButtonBN::isMeetingRunning((string) $meeting_room->meeting_id, $url_val, $salt_val),
      		l(t('Edit'), 'admin/config/system/bigbluebutton/meeting_room/edit/' . $meeting_room->mid),
      		l(t('Test'), 'admin/config/system/bigbluebutton/meeting_room/test/' . $meeting_room->mid),
      		l(t('Delete'), 'admin/config/system/bigbluebutton/meeting_room/delete/' . $meeting_room->mid),
    	);
  	}

  	if (!$rows) {
    	$rows[] = array(array(
      		'data' => t('No meeting rooms available.'),
      		'colspan' => 5,
    		));
  	}

  	$build['meeting_rooms_table'] = array(
    	'#theme' => 'table',
    	'#header' => $header,
    	'#rows' => $rows,
  	);
  
  	return $build;

}

/**
 * Form builder for deleting a Meeting room.
 *
 */
function bigbluebutton_admin_meeting_room_delete_form($form, &$form_state,  $meeting_room_id = NULL) {

    $meeting_room = bigbluebutton_getMeeting_room($form_state['build_info']['args'][0]);

	$form['meeting_name'] = array(
    	'#type' => 'value',
    	'#value' => $meeting_room['meeting_name'],
 		 );

  	return confirm_form(
    	$form,
    	t('Are you sure you want to delete %meeting_room?', array('%meeting_name' => $meeting_room['meeting_name'])),
    	'admin/config/system/bigbluebutton/meeting_room',
    	t('This action cannot be undone.'),
    	t('Delete'),
    	t('Cancel')
  	);
}

/**
 * Submit handler for the confirm delete a Meeting room.
 *
 * @see contact_category_delete_form()
 */
function bigbluebutton_admin_meeting_room_delete_form_submit($form, &$form_state) {

	//debug($form_state, "form_state", $print_r = TRUE);

  	db_delete('meeting_room')
		->condition('mid', $form_state['build_info']['args'][0])
    	->execute();

  	drupal_set_message(t('Meeting room %meeting_name has been deleted.', array('%meeting_name' => $form_state['values']['meeting_name'])));

  	$form_state['redirect'] = 'admin/config/system/bigbluebutton/meeting_room';

}

function bigbluebutton_admin_meeting_room_test($form, &$form_state) {

        $meeting_room = bigbluebutton_getMeeting_room($form_state['build_info']['args'][0]);

	$user = $GLOBALS['user'];
	$meeting_room['user_name'] = $user->name;
	$meeting_room['user_password'] = $meeting_room['moderator_pass'];


	if ( ($bbb_joinURL = bigbluebutton_joinMeetingRoom($meeting_room)) != NULL )
		header('Location: '.$bbb_joinURL );
}

function bigbluebutton_getMeeting_room($mid = NULL) {

    $meeting_room = array(
        'mid' => '',
        'meeting_id' => '',
        'meeting_name' => '',
    	'attendee_pass' => '',
    	'moderator_pass' => '',
    	'wait_for_moderator' => 0,
    	'newwindow' => 0,
    	'record' => 0,
    	'description' => '',
    	'welcome' => '',
  	);
	
    if ( $mid != NULL ) {		
        $results = db_select('meeting_room', 'm')
            ->condition('mid', $mid)
            ->fields('m', array('mid', 'meeting_id', 'meeting_name', 'attendee_pass', 'moderator_pass', 'wait_for_moderator', 'newwindow', 'record', 'description', 'welcome'))
            ->orderBy('meeting_name')
            ->execute()
            ->fetchAll();
    
            if ( $results != NULL ) {
                $meeting_room = array(
                    'mid' => $results[0]->mid,
                    'meeting_id' => $results[0]->meeting_id,
                    'meeting_name' => $results[0]->meeting_name,
                    'attendee_pass' => $results[0]->attendee_pass,
                    'moderator_pass' => $results[0]->moderator_pass,
                    'wait_for_moderator' => $results[0]->wait_for_moderator,
                    'newwindow' => $results[0]->newwindow,
                    'record' => $results[0]->record,
                    'description' => $results[0]->description,
                    'welcome' => $results[0]->welcome,
                    );
            }
    }

    return $meeting_room;
    
}


function bigbluebutton_joinMeetingRoom($meeting_room){
    //---------------------------------------------------JOIN-----------------------------------------------
    $bbb_joinURL = NULL;
	
    $meetingID = $meeting_room['meeting_id'];
    $meetingName = $meeting_room['meeting_name'];
    $moderatorPW = $meeting_room['moderator_pass'];
    $attendeePW = $meeting_room['attendee_pass'];
    $welcome = $meeting_room['welcome'];
    if( !isset($welcome) || $welcome == '')
	$welcome = "Welcome to [$meetingName] Meeting Room.";
    $record = $meeting_room['record'] == 1? 'true':'false';
    $description = $meeting_room['description'];
    
    $url_val = variable_get('bigbluebutton_server_url', '');
    $salt_val = variable_get('bigbluebutton_server_salt', '');
    $logoutURL = $GLOBALS['base_url'];

    $user_name = $meeting_room['user_name'];
    $user_password = $meeting_room['user_password'];

    //Validate the password
    if ( $user_password != $moderatorPW && $user_password != $attendeePW ) {
        drupal_set_message(t("The password typed doesn't match neither the attendee password nor the moderator password registered."));

    } else {
        //Calls create meeting on the bigbluebutton server
	if ( !BigBlueButtonBN::isMeetingCreated( $meetingID, $url_val, $salt_val ) ){
            $response = BigBlueButtonBN::createMeetingArray($meetingName, $meetingID, $welcome, $moderatorPW, $attendeePW, $salt_val, $url_val, $logoutURL, $record, array("meta_description" => $description, "meta_email" => '') );
		
            //Analyzes the bigbluebutton server's response
            if(!$response){//If the server is unreachable, then prompts the user of the necessary action
                drupal_set_message(t('Unable to join the meeting room %meeting_name. Please check the url of the bigbluebutton server AND check to see if the bigbluebutton server is running.', array('%meeting_name' => $meeting_room['meeting_name'])));
                return;
            } else if( $response['returncode'] == 'FAILED' ) { //The meeting was not created
                if($response['messageKey'] == 'idNotUnique'){
                    $createNew = true;
                } else if($response['messageKey'] == 'checksumError'){
                    drupal_set_message(t('A checksum error occured. Make sure you entered the correct salt.'));
                } else {
                    drupal_set_message(t($response['message']));
                }
                return;
            } else
                $bbb_joinURL = BigBlueButtonBN::joinURL($meetingID, $user_name, $user_password, $salt_val, $url_val, $logoutURL );
	} else  
            $bbb_joinURL = BigBlueButtonBN::joinURL($meetingID, $user_name, $user_password, $salt_val, $url_val, $logoutURL );
    }
    //---------------------------------------------------END-------------------------------------------------
    return $bbb_joinURL;
}


function bigbluebutton_getAllMeetingRooms() {

    $results = db_select('meeting_room', 'm')
        ->fields('m', array('mid', 'meeting_id', 'meeting_name', 'attendee_pass', 'moderator_pass', 'wait_for_moderator', 'newwindow', 'record', 'description', 'welcome'))
        ->orderBy('meeting_name')
        ->execute()
        ->fetchAll();

    foreach( $results as $rowset )
  	$meeting_rooms[$rowset->mid] = array(
            'mid' => $rowset->mid,
            'meeting_id' => $rowset->meeting_id,
            'meeting_name' => $rowset->meeting_name,
            'attendee_pass' => $rowset->attendee_pass,
            'moderator_pass' => $rowset->moderator_pass,
            'wait_for_moderator' => $rowset->wait_for_moderator,
            'newwindow' => $rowset->newwindow,
            'record' => $rowset->record,
            'description' => $rowset->description,
            'welcome' => $rowset->welcome,
            );

    return $meeting_rooms;
}


?>