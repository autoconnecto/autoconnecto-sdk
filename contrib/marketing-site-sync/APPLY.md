# Marketing site update bundle

These files mirror changes for **`autoconnecto/website`** (www.autoconnecto.in). The Cloud Agent token cannot push to the website repo; apply this bundle manually or via PR.

## What changed

- **New:** `SolutionsSection.tsx`, `ProductSolutionsSection.tsx` — Machine Fleet, Generator Monitoring, edge gateways, OTA/LTE/STM32 highlights
- **Updated:** homepage, product page, header nav (`#solutions`), hero, features, scale industries, `llms.txt`, `ai.txt`

## Apply to website repo

```bash
git clone https://github.com/autoconnecto/website.git
cd website
git checkout -b cursor/update-platform-marketing-bac4

# From autoconnecto-sdk repo root:
cp contrib/marketing-site-sync/src/app/components/* src/app/components/
cp contrib/marketing-site-sync/src/app/page.tsx src/app/
cp contrib/marketing-site-sync/src/app/product/page.tsx src/app/product/
cp contrib/marketing-site-sync/src/components/Header.tsx src/components/
cp contrib/marketing-site-sync/public/llms.txt contrib/marketing-site-sync/public/ai.txt public/

npm run type-check
git add -A && git commit -m "Add Solutions section and latest platform developments"
git push -u origin cursor/update-platform-marketing-bac4
```

Then merge and deploy (GitHub Actions → S3/CloudFront).

## Docs site (`autoconnecto-docs`)

Copy `docs/vitepress/` from this SDK repo into the private docs repo, update VitePress sidebar, and run the public docs deploy workflow.
