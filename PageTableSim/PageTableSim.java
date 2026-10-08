/*
*
 * Reads a page table and a list of logical memory requests from a file,
 * then translates each logical address into a physical address.
 */

import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;

public class PageTableSim {

    private static final int VALID_BIT = 1 << 18;
    private static final int READONLY_BIT = 1 << 17;
    private static final int SHARED_BIT = 1 << 16;
    private static final int FRAME_MASK = 0xFFFF;
    private static final int OFFSET_BITS = 8;
    private static final int OFFSET_MASK = 0xFF;

    public static void main(String[] args) {
        Scanner console = new Scanner(System.in);

        System.out.print("Enter a page table file: ");
        System.out.flush();
        if (!console.hasNext()) {
            System.err.println("Error: no filename entered.");
            System.exit(1);
        }
        String filename = console.next();

        Scanner in;
        try {
            in = new Scanner(new File(filename));
        } catch (FileNotFoundException e) {
            System.err.printf("Error: could not open file '%s'.%n", filename);
            System.exit(1);
            return;
        }

        try {
            int size;
            try {
                String sizeStr = in.next();
                size = Integer.parseInt(sizeStr);
            } catch (Exception e) {
                size = -1;
            }
            if (size < 2 || size > (1 << 16) || (size & (size - 1)) != 0) {
                System.err.println("Error: invalid page table size.");
                System.exit(1);
            }

            int pageMask = size - 1;

            int[] table = new int[size];
            for (int i = 0; i < size; i++) {
                try {
                    String entryStr = in.next();
                    table[i] = Integer.parseInt(entryStr, 16);
                } catch (Exception e) {
                    System.err.printf("Error: could not read page table entry %d.%n", i);
                    System.exit(1);
                }
            }

            int n;
            try {
                String countStr = in.next();
                n = Integer.parseInt(countStr);
            } catch (Exception e) {
                System.err.println("Error: could not read number of requests.");
                System.exit(1);
                return;
            }

            for (int i = 0; i < n; i++) {
                int request;
                try {
                    String requestStr = in.next();
                    request = Integer.parseInt(requestStr, 16);
                } catch (Exception e) {
                    System.err.printf("Error: could not read request %d.%n", i);
                    break;
                }

                int offset = request & OFFSET_MASK;
                int page = (request >>> OFFSET_BITS) & pageMask;
                int entry = table[page];

                System.out.printf("%nRequest: 0x%08X%n", request);
                System.out.printf("Page table entry: 0x%08X%n", entry);

                if ((entry & VALID_BIT) == 0) {
                    System.out.println("INVALID");
                    continue;
                }

                int frame = entry & FRAME_MASK;
                int physical = (frame << OFFSET_BITS) | offset;

                System.out.printf("Physical Address: 0x%08X%n", physical);
                System.out.printf("Frame Number: 0x%04X%n", frame);
                System.out.printf("Offset: 0x%02X%n", offset);
                System.out.printf("Read only: %s%n", (entry & READONLY_BIT) != 0 ? "TRUE" : "FALSE");
                System.out.printf("Shared: %s%n", (entry & SHARED_BIT) != 0 ? "TRUE" : "FALSE");
            }
        } finally {
            in.close();
        }
    }
}
