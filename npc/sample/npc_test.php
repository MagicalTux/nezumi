<?php
/* First attempt of a PHP-based NPC
 * $Id$
 * By MagicalTux <MagicalTux@ookoo.org>
 */

$Nezumi->AddNPC('MagicalTux_20060203_TestNpc', 'prontera.gat', 156, 191, 'Test NPC', 94, 6);

class MagicalTux_20060203_TestNpc {
	function __construct() {
		$Nezumi->mes("[Test]"); /* $Nezumi->mes() will be replaced by echo */
		$Nezumi->mes("This is a test");
		$Nezumi->Close();
	}
}

