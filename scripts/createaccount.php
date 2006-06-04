﻿<?php
// With this code, you have a HTML page to create an account
// *********************************************************

// note: player must confirm password, and its e-mail if you want better protection (default)
//       This php need only 'class ladmin' included in 'nezumi.php'.

// By DarkRaven

	// -------------------
	// ---- Options (please modify desired values):
	$Language = 'English'; // possible: 'English' (default), 'French', 'Russian', 'Japanese'
	$css_style = '<your style.css url>'; // Set here the link to add a style (used in <link rel="stylesheet" href="MylCastel.css" type="text/css">)
	$AddEmail = 1; // 0: doesn't create account with email, 1: Create account with email (default)
//	for future extension
//	$SetGMlevel = 'N'; // Set here if you want set GM level of the player (default: 'N'; values 'Y' or 'N')
//	$GMlevel = 0; // Set here the default GM level of the account (default: 0; values from 0 to 99!)
	$serverip = '127.0.0.1'; // Set here ip of the login server (default: 127.0.0.1)
	$serverport = 6900; // Set here port of your login-server (default: 6900)
	$adminpasswd = '<your password>'; // Set here ladmin password of your server
	// ---- End of options
	// -------------------

	// Texts of each language
	// ----------------------
	// To add a language: Add a field in the '$lang' array with appropriate text
	// note: some language definitions (international charset UTF-8 is used for all language):
	//       english: Lang = en
	//       russian: Lang = ru
	//       french: Lang = fr
	//       german: Lang = de
	//       greek: Lang = el
	//       hebrew: Lang = iw
	//       chinese: Lang = zh
	//       korean: Lang = ko
	//       japanese: Lang = ja
$lang = array (
	'English' => array (
		// Language definition
		'Lang' => 'en',
		'Display' => 'To display in English, click here.',
		// Title
		'Title' => 'ACCOUNT CREATION',
		// Maintenance
		'Maintenance' => 'Impossible to communicate with server. Sorry, you can not create your account now.',
		// Form fields
		'text' => 'Please, enter all values to create your account:',
		'account' => 'Account name',
		'sex' => 'Sex',
		'sex1' => 'Male',
		'sex2' => 'Female',
		'email' => 'Email of the account',
		'pass' => 'Password',
		'conf_pass' => 'Confirm Password',
		// Form buttons
		'Submit' => 'Submit',
		'Reset' => 'Reset',
		// Errors
		'bad_account' => 'Invalid account name (It must have between 4 to 24 characters).',
		'bad_sex' => 'Invalid sex (Your sex must be Male or Female).',
		'bad_email' => 'Invalid email (It must have between 3 to 24 characters).',
		'bad_email2' => 'Invalid email (It must have a @).',
		'bad_email3' => 'Invalid email (\'@\' can not be latest character).',
		'bad_email4' => 'Invalid email (\'.\' can not be latest character).',
		'bad_email5' => 'Invalid email (The last \'@\' can not be followed by a \'.\').',
		'bad_email6' => 'Invalid email (After the last \'@\' you can not have \'..\').',
		'bad_email7' => 'Invalid email (After the last \'@\' you can not have a \' (space)\').',
		'bad_email8' => 'Invalid email (After the last \'@\' you can not have a \';\').',
		'bad_pass' => 'Invalid password (It must have between 4 to 24 characters).',
		'bad_conf_pass' => 'Invalid confirmed password (It must have between 4 to 24 characters).',
		'bad_conf_pass2' => 'Invalid confirmed password (password and confirmed password must be same).',
		// Result
		'Success' => 'Account \'%s\' successfully created.',
		'Failed' => 'Your account can not be created. (Possible reasons: Account name already exists, or you use incorrect characters in account name).',
		// Copyright
		'Copyright' => 'Powered by Darkraven for Nezumi Project.',
	),
	'French' => array (
		// Language definition
		'Lang' => 'fr',
		'Display' => 'Pour afficher en Français, cliquez ici.',
		// Title
		'Title' => 'CREATION D\'UN COMPTE',
		// Maintenance
		'Maintenance' => 'Impossible de communiquer avec le serveur. Désolé, vous ne pouvez pas créer votre compte maintenant.',
		// Form fields
		'text' => 'Merci d\'entrer les différentes valeurs pour créer votre compte:',
		'account' => 'Nom du compte',
		'sex' => 'Sexe',
		'sex1' => 'Homme',
		'sex2' => 'Femme',
		'email' => 'Email du compte',
		'pass' => 'Mot de passe',
		'conf_pass' => 'Confirmation du mot de passe',
		// Form buttons
		'Submit' => 'Envoyer',
		'Reset' => 'Réinitialiser',
		// Errors
		'bad_account' => 'Nom de compte incorrect (Il doit avoir entre 4 et 24 caractères).',
		'bad_sex' => 'Sexe incorrect (Votre sexe doit être Homme ou Femme).',
		'bad_email' => 'Email incorrect (Il doit avoir entre 3 et 24 caractères).',
		'bad_email2' => 'Email incorrect (Il doit avoir un @).',
		'bad_email3' => 'Email incorrect (Le \'@\' ne peut pas être le dernier caractère).',
		'bad_email4' => 'Email incorrect (L\'email ne peut pas se terminer par un \'.\').',
		'bad_email5' => 'Email incorrect (Le dernier \'@\' ne peut pas être suivi par un \'.\').',
		'bad_email6' => 'Email incorrect (Après le dernier \'@\' vous ne pouvez pas avoir \'..\').',
		'bad_email7' => 'Email incorrect (Après le dernier \'@\' vous ne pouvez pas avoir un \' (espace)\').',
		'bad_email8' => 'Email incorrect (Après le dernier \'@\' vous ne pouvez pas avoir un \';\').',
		'bad_pass' => 'Mot de passe incorrect (Il doit avoir entre 4 et 24 caractères).',
		'bad_conf_pass' => 'Confirmation du mot de passe incorrect (Il doit avoir entre 4 et 24 caractères).',
		'bad_conf_pass2' => 'Confirmation du mot de passe incorrect (Il doit identique au mot de passe).',
		// Result
		'Success' => 'Compte \'%s\' créé avec succès.',
		'Failed' => 'Votre compte ne peut pas être créé. (Raisons possibles: Le nom du compte existe déjà, ou vous utilisez des caractères incorrects dans le nom du compte).',
		// Copyright
		'Copyright' => 'Créé par Darkraven pour le Projet Nézumi.',
	),
	'Russian' => array (
		// Language definition
		'Lang' => 'ru',
		'Display' => 'Для выбора русского языка нажмите здесь.',
		// Title
		'Title' => 'Создание учетной записи',
		// Maintenance
		'Maintenance' => 'Невозможно подключиться к серверу, вы не можете создать учетную запись сейчас.',
		// Form fields
		'text' => 'Пожалуйста, укажите все значения полей:',
		'account' => 'Имя учетной записи',
		'sex' => 'Пол',
		'sex1' => 'Мужской',
		'sex2' => 'Женский',
		'email' => 'Адрес электронной почты',
		'pass' => 'Пароль',
		'conf_pass' => 'Подтверждение',
		// Form buttons
		'Submit' => 'Отправить',
		'Reset' => 'Сброс',
		// Errors
		'bad_account' => 'Неправильное имя учетной записи (должно быть не менее 4 и не более 24 символов).',
		'bad_sex' => 'Неправильный пол (должен быть мужской или женский).',
		'bad_email' => 'Неправильный адрес электронной почты (должно быть не менее 3 и не более 24 символов).',
		'bad_email2' => 'Неправильный адрес электронной почты (должно содержать символ @).',
		'bad_email3' => 'Неправильный адрес электронной почты (\'@\' не может быть последним символом).',
		'bad_email4' => 'Неправильный адрес электронной почты (\'.\' не может быть последним символом).',
		'bad_email5' => 'Неправильный адрес электронной почты (символ \'@\' не может продолжаться символом \'.\').',
		'bad_email6' => 'Неправильный адрес электронной почты (после символа \'@\' не может быть символа \'..\').',
		'bad_email7' => 'Неправильный адрес электронной почты (после символа \'@\' не может быть символа \'(пробел)\').',
		'bad_email8' => 'Неправильный адрес электронной почты (после символа \'@\' не может быть символа \';\').',
		'bad_pass' => 'Неправильный пароль (должен быть не менее 4 и не более 24 символов).',
		'bad_conf_pass' => 'Неправильное подтверждение пароля (должен быть не менее 4 и не более 24 символов).',
		'bad_conf_pass2' => 'Неправильное подтверждение пароля (пароль и подтверждение должны быть одинаковыми).',
		// Result
		'Success' => 'Учетная запись \'%s\' успешно создана.',
		'Failed' => 'Учетная запись не была создана (возможные причины: учетная запись уже существует, запрещенные символы в имени учетной записи).',
		// Copyright
		'Copyright' => 'Автор: Darkraven для Nezumi Project.',
	),
	'Japanese' => array (
		// Language definition
		'Lang' => 'ja',
		'Display' => '日本語表示',
		// Title
		'Title' => 'アカウント作成',
		// Maintenance
		'Maintenance' => 'サーバーとの接続に失敗',
		// Form fields
		'text' => '全ての項目を埋めてください',
		'account' => 'アカウントID',
		'sex' => '性別',
		'sex1' => '♂',
		'sex2' => '♀',
		'email' => 'メールアドレス',
		'pass' => 'パスワード',
		'conf_pass' => 'パスワード（確認）',
		// Form buttons
		'Submit' => '作成',
		'Reset' => 'やり直し',
		// Errors
		'bad_account' => 'アカウントIDが無効',
		'bad_sex' => '性別が無効',
		'bad_email' => 'メールアドレスが無効',
		'bad_email2' => 'メールアドレスが無効',
		'bad_email3' => 'メールアドレスが無効',
		'bad_email4' => 'メールアドレスが無効',
		'bad_email5' => 'メールアドレスが無効',
		'bad_email6' => 'メールアドレスが無効',
		'bad_email7' => 'メールアドレスが無効',
		'bad_email8' => 'メールアドレスが無効',
		'bad_pass' => 'パスワードが無効',
		'bad_conf_pass' => 'パスワード（確認）が無効',
		'bad_conf_pass2' => 'パスワードとパスワード（確認）の内容が違う',
		// Result
		'Success' => 'アカウント \'%s\' の作成に成功',
		'Failed' => 'アカウント作成に失敗(同名のアカウントが存在するか、不正な文字が含まれている)',
		// Copyright
		'Copyright' => 'Powered by Darkraven for Nezumi Project.',
	),
);
	
	// ---- End of languages texts
	// ---------------------------

// ------------ after this line, you change code, not only options.

	// check of http datas
	function NormalisationHHTP($tableau) {
		foreach($tableau as $cle=>$valeur) {
			if (!is_array($valeur)) { // It's a element, we act
				$tableau[$cle] = stripslashes($valeur);
			} else { // it's an array, we call recursivly
				$tableau[$cle] = NormalisationHHTP($valeur);
			}
		}
		return $tableau;
	}
	if (get_magic_quotes_gpc()) {
		$_POST = NormalisationHHTP($_POST);
		$_GET  = NormalisationHHTP($_GET);
	}

	// Get transmited language if it exists
	if (isset($_POST['language'])) {
		$Language = $_POST['language'];
	} else if (isset($_GET['language'])) {
		$Language = $_GET['language'];
	}
	// check value of the language
	if (!isset($lang[$Language])) {
		$Language = 'English'; // Default value
	}
	// check $GMlevel of the account
	if (!isset($GMlevel) || $GMlevel < 0 || $GMlevel > 99) {
		$GMlevel = 0; // Default value
	}

	// Get sended values
	if (isset($_POST['account'])) {
		$account = $_POST['account'];
	} else {
		$account = "";
	}
	if (isset($_POST['sex'])) {
		$sex = $_POST['sex'];
	} else {
		$sex = "";
	}
	if (isset($_POST['email'])) {
		$email = $_POST['email'];
	} else {
		$email = "";
	}
	if (isset($_POST['pass'])) {
		$pass = $_POST['pass'];
	} else {
		$pass = "";
	}
	if (isset($_POST['conf_pass'])) {
		$conf_pass = $_POST['conf_pass'];
	} else {
		$conf_pass = "";
	}

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>

<head>

	<meta http-equiv="content-type" content="text/html; charset=UTF-8">
	<?php
		if (isset($lang[$Language]['Lang'])) {
			echo '<meta http-equiv="Content-language" content="'.$lang[$Language]['Lang'].'">'."\n";
		} else {
			echo '<meta http-equiv="Content-language" content="en">'."\n";
		}
	?>
	<meta http-equiv="Content-Style-Type" content="text/css">
	<meta http-equiv="imagetoolbar" content="no">
	<meta content="Darkraven" name="author">

	<?php
		if (isset($css_style)) {
			echo '<link rel="stylesheet" href="'.$css_style.'" type="text/css">'."\n";
		}
	?>

	<title>Nezumi - <?php echo htmlspecialchars($lang[$Language]['Title'], ENT_QUOTES) ?></title>

</head>

<SCRIPT type="text/javascript">
<!--
	function SubmitValues() {
		var last_arobas = document.account.email.value.indexOf("@");
		while (document.account.email.value.indexOf("@", last_arobas + 1) != -1) {
			last_arobas = document.account.email.value.indexOf("@", last_arobas + 1);
		}
		if (document.account.account.value.length < 4 || document.account.account.value.length > 24) {
			document.getElementById('Error').innerHTML = <?php echo "\"".htmlspecialchars($lang[$Language]['bad_account'], ENT_QUOTES)."\""; ?>;
			return false;
		// check about Sex is unnecessary in javascript
<?php if ($AddEmail) { ?>
		} else if (document.account.email.value.length < 3 || document.account.email.value.length > 40) {
			document.getElementById('Error').innerHTML = <?php echo "\"".htmlspecialchars($lang[$Language]['bad_email'], ENT_QUOTES)."\""; ?>;
			return false;
		} else if (last_arobas == -1) {
			document.getElementById('Error').innerHTML = <?php echo "\"".htmlspecialchars($lang[$Language]['bad_email2'], ENT_QUOTES)."\""; ?>;
				return false;
		} else if ((last_arobas + 1) == document.account.email.value.length) {
				document.getElementById('Error').innerHTML = <?php echo "\"".htmlspecialchars($lang[$Language]['bad_email3'], ENT_QUOTES)."\""; ?>;
				return false;
		} else if (document.account.email.value.charAt(document.account.email.value.length - 1) == '.') {
				document.getElementById('Error').innerHTML = <?php echo "\"".htmlspecialchars($lang[$Language]['bad_email4'], ENT_QUOTES)."\""; ?>;
				return false;
		} else if (document.account.email.value.charAt(last_arobas + 1) == '.') {
				document.getElementById('Error').innerHTML = <?php echo "\"".htmlspecialchars($lang[$Language]['bad_email5'], ENT_QUOTES)."\""; ?>;
				return false;
		} else if (document.account.email.value.indexOf("..", last_arobas + 1) != -1) {
				document.getElementById('Error').innerHTML = <?php echo "\"".htmlspecialchars($lang[$Language]['bad_email6'], ENT_QUOTES)."\""; ?>;
				return false;
		} else if (document.account.email.value.indexOf(" ", last_arobas + 1) != -1) {
				document.getElementById('Error').innerHTML = <?php echo "\"".htmlspecialchars($lang[$Language]['bad_email7'], ENT_QUOTES)."\""; ?>;
				return false;
		} else if (document.account.email.value.indexOf(";", last_arobas + 1) != -1) {
				document.getElementById('Error').innerHTML = <?php echo "\"".htmlspecialchars($lang[$Language]['bad_email8'], ENT_QUOTES)."\""; ?>;
				return false;
<?php } ?>
		} else if (document.account.pass.value.length < 4 || document.account.pass.value.length > 24) {
			document.getElementById('Error').innerHTML = <?php echo "\"".htmlspecialchars($lang[$Language]['bad_pass'], ENT_QUOTES)."\""; ?>;
			return false;
		} else if (document.account.conf_pass.value.length < 4 || document.account.conf_pass.value.length > 24) {
			document.getElementById('Error').innerHTML = <?php echo "\"".htmlspecialchars($lang[$Language]['bad_conf_pass'], ENT_QUOTES)."\""; ?>;
			return false;
		} else if (document.account.pass.value != document.account.conf_pass.value) {
			document.getElementById('Error').innerHTML = <?php echo "\"".htmlspecialchars($lang[$Language]['bad_conf_pass2'], ENT_QUOTES)."\""; ?>;
			return false;
		} else {
			return true;
		}
	}

	function ResetValues() {
		document.account.account.value = "";
<?php if ($AddEmail) { ?>
		document.account.email.value = "";
<?php } ?>
		document.account.pass.value = "";
		document.account.conf_pass.value = "";
		return true;
	}
// -->
</SCRIPT>

<body>

	<table align="center" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td align="center" valign="top" width="100%%">

<?php
	// If include doesn't exist!
	if (!file_exists('nezumi.php')) {
		// display simple page for maintenance
printf('
			<h3>'.htmlspecialchars($lang[$Language]['Maintenance'], ENT_QUOTES).'</h3>
');

	} else {
		include('nezumi.php');
?>

<?php
	// don't display an error by default
	$error_message = "";
	// by default, account is not created
	$result = false;
	// if player send form, check values
	if ($account != "" || $sex != "" || ($AddEmail && $email != "") || $pass != "" || $conf_pass != "") {
		if ($AddEmail) {
			$email_len = strlen($email);
			$last_arobas = strrchr($email, '@');
		}
		if (strlen($account) < 4 || strlen($account) > 24) {
			$error_message = $lang[$Language]['bad_account'];
		} else if ($sex != 'M' && $sex != 'F') {
			$error_message = $lang[$Language]['bad_sex'];
		} else if ($AddEmail && ($email_len < 3 || $email_len > 40)) {
			$error_message = $lang[$Language]['bad_email'];
		} else if ($AddEmail && $last_arobas == false) {
			$error_message = $lang[$Language]['bad_email2'];
		} else if ($AddEmail && $email[$email_len - 1] == '@') {
			$error_message = $lang[$Language]['bad_email3'];
		} else if ($AddEmail && $email[$email_len - 1] == '.') {
			$error_message = $lang[$Language]['bad_email4'];
		} else if ($AddEmail && $last_arobas[1] == '.') {
			$error_message = $lang[$Language]['bad_email5'];
		} else if ($AddEmail && strstr($last_arobas, "..") != false) {
			$error_message = $lang[$Language]['bad_email6'];
		} else if ($AddEmail && strchr($last_arobas, " ") != false) {
			$error_message = $lang[$Language]['bad_email7'];
		} else if ($AddEmail && strchr($last_arobas, ";") != false) {
			$error_message = $lang[$Language]['bad_email8'];
		} else if (strlen($pass) < 4 || strlen($pass) > 24) {
			$error_message = $lang[$Language]['bad_pass'];
		} else if (strlen($conf_pass) < 4 || strlen($conf_pass) > 24) {
			$error_message = $lang[$Language]['bad_conf_pass'];
		} else if (strcmp($pass, $conf_pass) != 0) {
			$error_message = $lang[$Language]['bad_conf_pass2'];
		}
	}

	// if no error (datas seems valid)
	if ($error_message == "" &&
	    ($account != "" || $sex != "" || ($AddEmail && $email != "") || $pass != "" || $conf_pass != "")) {
		// connect to server
		$ladmin_conect = new ladmin;
		if (!$ladmin_conect->connect($serverip, $adminpasswd, $serverport)) {
			$error_message = $lang[$Language]['Maintenance'];
		} else {
			// Create the account and get result
			$result = $ladmin_conect->make_account($account, $pass, $sex, ($AddEmail) ? $email : "");
			if ($result == false) {
				$error_message = $lang[$Language]['Failed'];
			} else {
//	for futur extension
//				if ($SetGMlevel == 'Y') {
//					// try to change GM level
//					$ladmin_conect->changeGMlevel($account, $GMlevel);
//					// don't display any answer about this modification
//				}
			}
		}
		$ladmin_conect->close(); // close in all case (password can be incorrect and connection opened)
	}

	if ($result != false) {
		$success = sprintf($lang[$Language]['Success'], $account);
printf('
			<h3>'.htmlspecialchars($success, ENT_QUOTES).'</h3>
');

	} else {
?>

			<table border="0" cellpadding="0" cellspacing="0" width="100%">
			<tr>
				<td align="center" valign="top">
					<?php
					// Display title
					print('<h2>'.htmlspecialchars($lang[$Language]['Title'], ENT_QUOTES).'</h2>'."\n");

					// display link to change language
					foreach($lang as $i => $j) {
						if ($i != $Language && isset($j['Display'])) {
							if (isset($j['Lang'])) {
								printf("\t\t\t\t\t".'<a href="'.$_SERVER['PHP_SELF'].'?language='.$i.'" lang="'.$j['Lang'].'">'.htmlspecialchars($j['Display'], ENT_QUOTES).'</a><br />'."\n");
							} else {
								printf("\t\t\t\t\t".'<a href="'.$_SERVER['PHP_SELF'].'?language='.$i.'">'.htmlspecialchars($j['Display'], ENT_QUOTES).'</a><br />'."\n");
							}
						}
					}
					printf("\t\t\t\t\t".'<br />'."\n");

					// here begin de form
					print("\t\t\t\t\t".'<form name="account" method="post" action="'.$_SERVER['PHP_SELF'].'?language='.$Language.'" onSubmit="return SubmitValues();">
						<table align="center" border="0" cellpadding="3" cellspacing="0">
						<tr align="center" >
							<td colspan="2" align="center">
								<br />
								'.htmlspecialchars($lang[$Language]['text'], ENT_QUOTES).'
							</td>
						</tr>

						<tr align="center">
							<td align="right">'.htmlspecialchars($lang[$Language]['account'], ENT_QUOTES).' :</td>
							<td align="left"><input type="text" name="account" value="'.htmlspecialchars($account, ENT_QUOTES).'" size="24" maxlength="24"></td>
						</tr>
						<tr align="center">
							<td align="right">'.htmlspecialchars($lang[$Language]['sex'], ENT_QUOTES).' :</td>
							<td align="left">'."\n");
							if (isset($sex) && $sex == 'F') {
								print("\t\t\t\t\t\t".'<input type="radio" name="sex" value="M" />'.htmlspecialchars($lang[$Language]['sex1'], ENT_QUOTES).'&nbsp;&nbsp;
								<input type="radio" name="sex" value="F" checked />'.htmlspecialchars($lang[$Language]['sex2'], ENT_QUOTES)."\n");
							} else {
								print("\t\t\t\t\t\t".'<input type="radio" name="sex" value="M" checked />'.htmlspecialchars($lang[$Language]['sex1'], ENT_QUOTES).'&nbsp;&nbsp;
								<input type="radio" name="sex" value="F" />'.htmlspecialchars($lang[$Language]['sex2'], ENT_QUOTES)."\n");
							}
						print("\t\t\t\t\t\t".'</td>
						</tr>'."\n");
if ($AddEmail) {
						print("\t\t\t\t\t".'<tr align="center">
							<td align="right">'.htmlspecialchars($lang[$Language]['email'], ENT_QUOTES).' :</td>
							<td align="left"><input type="text" name="email" value="'.htmlspecialchars($email, ENT_QUOTES).'" size="40" maxlength="40"></td>
						</tr>'."\n");
}
						print("\t\t\t\t\t".'<tr align="center">
							<td align="right">'.htmlspecialchars($lang[$Language]['pass'], ENT_QUOTES).' :</td>
							<td align="left"><input type="password" name="pass" value="" size="24" maxlength="24"></td>
						</tr>
						<tr align="center">
							<td align="right">'.htmlspecialchars($lang[$Language]['conf_pass'], ENT_QUOTES).' :</td>
							<td align="left"><input type="password" name="conf_pass" value="" size="24" maxlength="24"></td>
						</tr>
						<tr align="center">
							<td colspan="2" align="center">
								<i><span id="Error" style="display:inline; color:red">'.htmlspecialchars($error_message, ENT_QUOTES).'</span></i>
							</td>
						</tr>
						</table>

						<br />
						<input type="submit" name="submit" value="'.htmlspecialchars($lang[$Language]['Submit'], ENT_QUOTES).'" />&nbsp;&nbsp;<input type="button" value="'.htmlspecialchars($lang[$Language]['Reset'], ENT_QUOTES).'" name="reset" onClick="ResetValues();" />
					</form>'."\n");
					?>
				</td>
			</tr>
			</table>
<?php
	} // if ($result == true) {
} // if (!file_exists('nezumi.php')) {
?>
		</td>
	</tr>
	</table>

	<table align="center" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td align="center">
			<br />
			<?php
			// display copyright here
			echo '<i>'.htmlspecialchars($lang[$Language]['Copyright'], ENT_QUOTES).'</i><br />'."\n";
			?>
		</td>
	</tr>
	</table>

</body>

</html>
