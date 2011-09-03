/* Levelize
	this program is for Andradion 2 development of levels,
	it translates the ASCII data of a level to a binary
	layout.  For example, if a palette entry in the ASCII
	file is listed "255 255 128" then Levelize will write
	the bytes 0xff, 0xff, and 0x80 in binary to the output
	file.  The arguments to this program drive it; they should
	be a series of 2-character strings that identify the levels
	to levelize. */

import java.io.*;

public class Levelize
{
    public static void main(String args[])
    {
	for(int argument_index = 0; argument_index < args.length; argument_index++)
	    {
		if(2 != args[argument_index].length())
		    {
			System.out.println("Argument #" + (1 + argument_index) + " is invalid: " + args[argument_index]);	
			continue;
		    }

		Transfer t = new Transfer(args[argument_index]);

		System.out.println("Starting work on: " + args[argument_index]);

		// transfer the data . . .
		// first the two palettes
		int q; // the Quantity of palette entries
		t.Byte(3 * (0 == (q = t.Byte()) ? 256 : q));
		t.Byte(3 * (0 == (q = t.Byte()) ? 256 : q));

		// transfer one coordinate, a script index, and another coordinate
		t.Word(2); t.Byte(); t.Word(2);

		// do indoor rectangles
		t.Word(t.Byte() * 4); 

		// do level ends
		q = t.Byte();
		for(int i = 0; i < q; i++) {t.Byte(); t.Word(2);}

		// do the three enemy types, the three weapon powerup types, and the health powerup (7 types of coor sets)
		for(int i = 0; i< 7; i++) {t.Word(t.Byte() * 2);}
	    }
    }
}

class Transfer
{
    private InputStream inf;
    private OutputStream outf;

    public Transfer(String level_id)
    {
	try
	    {
		inf = new FileInputStream("c:\\Andradion 2\\DatSrc\\" + level_id + ".dat");
		outf = new FileOutputStream("c:\\Andradion 2\\Resource\\" + level_id + ".lvl",false);
	    }
	catch(FileNotFoundException fnfe)
	    {
		System.out.println("Argument " + level_id + " is invalid");
		System.exit(0);
	    }
	catch(SecurityException se)
	    {
		System.out.println("There was a security exception when openning level files of " + level_id);
		System.out.println(se);
	    }
    }

    // this function gets a number from the input stream that is typed
    //  in ASCII form
    private int GetNum()
    {
	int ch_read;
	do
	    {
		try
		    {
			ch_read = inf.read();
		    }
		catch(IOException ioe)
		    {
			System.out.println("There was an i/o exception:" + ioe);
			ch_read = -1;
		    }

		if(-1 == ch_read)
		    {
			// we reached the end of the file
			//  this shouldn't happen for correct
			//  level data files
			return 0;
		    }
	    }
	while(!Character.isDigit((char)ch_read));
	StringBuffer number = new StringBuffer();
	do
	    {
		number.append((char)ch_read);
		try
		    {
			ch_read = inf.read();
		    }
		catch(IOException ioe)
		    {
			ch_read = -1;
			System.out.println("There was an i/o exception: " + ioe);
		    }
	    }
	while(-1 != ch_read && Character.isDigit((char)ch_read));
	return Integer.parseInt(number.toString());
    }

    public int Byte()
    {
	int num = GetNum();
	try
	    {
		outf.write(num);
	    }
	catch(IOException ioe)
	    {
		System.out.println("There was an i/o exception: " + ioe);
	    }
	return num;
    }

    public int Word()
    {
	int num = GetNum();
	try
	    {
		outf.write(num & 0xff); // write less significant byte
		outf.write((num >> 8) & 0xff); // write more significant byte
	    }
	catch(IOException ioe)
	    {
		System.out.println("There was an i/o exception: " + ioe);
	    }
	return num;		  
    }

    public void Word(int times)
    {
	for(int i = 0; i < times; i++)
	    {
		Word();
	    }
    }

    public void Byte(int times)
    {
	for(int i  = 0; i < times; i++)
	    {
		Byte();
	    }
    }
}
