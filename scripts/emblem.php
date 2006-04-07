<?php
/*
	emblem.php v0.1 - By MagicalTux & LebrEf -
	Idea : LebrEf
	Code : MagicalTux (90%) - LebrEf (10%)

	Updates: evos
	> palette load, fread(); crashed the script
	> 8bits images were a pixel off to the right

*/

// Mysql Connection
// adress, login, password of your mysql server
$mysql_game = @mysql_connect( "localhost", "LOGIN", "PASSWORD" ) or die( "Can't connect to MySQL server [1]." );

// Your freya's database
$db_game = "DATABASE";

// Connexion ton mysql server
@mysql_select_db( $db_game, $mysql_game ) or die( "Can't connect to MySQL server [2]" );




function imagecreatefrombmpstring($im) {
	$header = unpack("vtype/Vsize/v2reserved/Voffset", substr($im,0,14));
	$info = unpack("Vsize/Vwidth/Vheight/vplanes/vbits/Vcompression/Vimagesize/Vxres/Vyres/Vncolor/Vimportant", substr($im,14,40));

	extract($info);
	extract($header);

	if($type != 0x4D42) return false;		  // signature "BM"

	$palette_size = $offset - 54;
	
	$ncolor = $palette_size / 4;									
//	$gd_header = "";
//	// true-color vs. palette
//	$gd_header .= ($palette_size == 0) ? "\xFF\xFE" : "\xFF\xFF";	  
//	$gd_header .= pack("n2", $width, $height);
//	$gd_header .= ($palette_size == 0) ? "\x01" : "\x00";
//	if($palette_size) $gd_header .= pack("n", $ncolor);
//	// no transparency
//	$gd_header .= "\xFF\xFF\xFF\xFF";	

	$imres=imagecreatetruecolor($width,$height);
	imagealphablending($imres,false);
	imagesavealpha($imres,true);
	$pal=array();

	if($palette_size) {
		$palette = substr($im, 54, $palette_size);
		$gd_palette = "";
		$j = 0; $n=0;
		while($j < $palette_size) {
			$b = ord($palette{$j++});
			$g = ord($palette{$j++});
			$r = ord($palette{$j++});
			$a = ord($palette{$j++});
			if ( ($r==255) && ($g==0) && ($b==255)) $a=127; // alpha = 255 on 0xFF00FF
			$pal[$n++]=imagecolorallocatealpha($imres, $r, $g, $b, $a);
		}
	}

	$scan_line_size = (($bits * $width) + 7) >> 3;
	$scan_line_align = ($scan_line_size & 0x03) ? 4 - ($scan_line_size & 0x03): 0;

	for($i = 0, $l = $height - 1; $i < $height; $i++, $l--) {
		// BMP stores scan lines starting from bottom
		$scan_line = substr($im, $offset + (($scan_line_size + $scan_line_align) * $l), $scan_line_size);
		if($bits == 24) {
			$j = 0; $n=0;
			while($j < $scan_line_size) {
				$b = ord($scan_line{$j++});
				$g = ord($scan_line{$j++});
				$r = ord($scan_line{$j++});
				$a = 0;
				if ( ($r==255) && ($g==0) && ($b==255)) $a=127; // alpha = 255 on 0xFF00FF
				$col=imagecolorallocatealpha($imres, $r, $g, $b, $a);
				imagesetpixel($imres, $n++, $i, $col);
			}
		}
		else if($bits == 8) {
			$j=0;
			while($j<$scan_line_size) {
				$col=$pal[ord($scan_line{$j++})];
				imagesetpixel($imres, $j-1, $i, $col);
			}
		}
		else if($bits == 4) {
			$j = 0; $n=0;
			while($j < $scan_line_size) {
				$byte = ord($scan_line{$j++});
				$p1 = $byte >> 4;
				$p2 = $byte & 0x0F;
				imagesetpixel($imres, $n++, $i, $pal[$p1]);
				imagesetpixel($imres, $n++, $i, $pal[$p2]);
			}
		}
		else if($bits == 1) {
			$j = 0; $n=0;
			while($j < $scan_line_size) {
				$byte = ord($scan_line{$j++});
				$p1 = (int) (($byte & 0x80) != 0);
				$p2 = (int) (($byte & 0x40) != 0);
				$p3 = (int) (($byte & 0x20) != 0);
				$p4 = (int) (($byte & 0x10) != 0);
				$p5 = (int) (($byte & 0x08) != 0);
				$p6 = (int) (($byte & 0x04) != 0);
				$p7 = (int) (($byte & 0x02) != 0);
				$p8 = (int) (($byte & 0x01) != 0);
				imagesetpixel($imres, $n++, $i, $pal[$p1]);
				imagesetpixel($imres, $n++, $i, $pal[$p2]);
				imagesetpixel($imres, $n++, $i, $pal[$p3]);
				imagesetpixel($imres, $n++, $i, $pal[$p4]);
				imagesetpixel($imres, $n++, $i, $pal[$p5]);
				imagesetpixel($imres, $n++, $i, $pal[$p6]);
				imagesetpixel($imres, $n++, $i, $pal[$p7]);
				imagesetpixel($imres, $n++, $i, $pal[$p8]);
			}
		}
	}
	return $imres;
}


if( isset( $_GET['guild'] ) && ereg( "^[0-9]+$", $_GET['guild'] ) )
{
	//Mysql query
	$query_emblem = mysql_query( "SELECT `emblem_data` FROM `guild` WHERE `guild_id` = '".$_GET['guild']."';", $mysql_game );
	$nb_emblem = mysql_num_rows( $query_emblem );
	$fetch_emblem = mysql_fetch_array( $query_emblem );
	$ebm=$fetch_emblem['emblem_data'];
	
	//Check... Emblem exist ?
	if( $nb_emblem != 0 && $ebm != 0 )
	{
		$ebm=@gzuncompress(pack('H*',$ebm));
		// need to convert that to PNG with transparency
		$im=imagecreatefrombmpstring($ebm);

		header('Content-Type: image/png');
		//The guild emblem ^_^
		imagepng($im);
		exit;		

	}
}

// Emblem failed to load - send empty image to prevent page from appearing broken
$im = imagecreate(24, 24);
$background_color = imagecolorallocatealpha ($im, 255, 0, 255,127);
imagecolortransparent($im, $background_color);

header ('Content-type: image/png');
imagepng($im);
exit;
?>